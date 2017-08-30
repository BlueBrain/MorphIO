
/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          bbp-open-source@googlegroups.com
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "synapse.h"
#include "detail/lockHDF5.h"

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Utility.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <lunchbox/log.h>
#include <lunchbox/scopedMutex.h>

#include <bitset>
#include <fstream>
#include <unordered_map>

namespace brion
{
namespace detail
{
namespace
{
struct Dataset
{
    std::unique_ptr<HighFive::DataSet> dataset;
    std::pair<size_t, size_t> dims;
};

bool _openDataset(const HighFive::File& file, const std::string& name,
                  Dataset& dataset)
{
    try
    {
        HighFive::SilenceHDF5 silence;
        dataset.dataset.reset(new HighFive::DataSet(file.getDataSet(name)));
    }
    catch (const HighFive::DataSetException&)
    {
        LBVERB << "Could not find synapse dataset for " << name << ": "
               << std::endl;
        return false;
    }

    auto dims = dataset.dataset->getSpace().getDimensions();

    if (dims.size() != 2)
    {
        LBERROR << "Synapse dataset is not 2 dimensional" << std::endl;
        return false;
    }
    dataset.dims = std::make_pair(dims[0], dims[1]);

    return true;
}
}
namespace fs = boost::filesystem;

/** Access a single synapse file (nrn*.h5 or nrn*.h5.<int> */
class SynapseFile : public boost::noncopyable
{
public:
    explicit SynapseFile(const std::string& source)
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());

        try
        {
            HighFive::SilenceHDF5 silence;
            _file.reset(new HighFive::File(source, H5F_ACC_RDONLY));
        }
        catch (const HighFive::FileException& exc)
        {
            LBTHROW(std::runtime_error("Could not open synapse file " + source +
                                       ": " + exc.what()));
        }

        Dataset dataset;
        const std::string& datasetName = _file->getObjectName(0);
        if (!detail::_openDataset(*_file, datasetName, dataset))
            LBTHROW(std::runtime_error("Cannot open dataset " + datasetName +
                                       " in synapse file " + source));

        _numAttributes = dataset.dims.second;
        if (_numAttributes != SYNAPSE_ALL &&
            _numAttributes != SYNAPSE_POSITION_ALL &&
            _numAttributes != SYNAPSE_OLD_POSITION_ALL &&
            _numAttributes != 1 /* nrn_extra */)
        {
            LBTHROW(std::runtime_error(source + " not a valid synapse file"));
        }
    }

    ~SynapseFile()
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        _file.reset();
    }

    template <size_t N>
    SynapseMatrix read(const uint32_t gid, const uint32_t attributes) const
    {
        const std::bitset<N> bits(attributes);
        if (!bits.any())
            return SynapseMatrix();

        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        Dataset dataset;
        if (!_openDataset(gid, dataset))
            return SynapseMatrix();

        std::vector<size_t> columns;
        columns.reserve(SYNAPSE_ALL);
        for (size_t i = 0; i < bits.size(); ++i)
        {
            if (bits.test(i))
                columns.push_back(i);
        }
        const auto selection = dataset.dataset->select(columns);

        SynapseMatrix values(boost::extents[dataset.dims.first][bits.count()]);
        selection.read(values);
        return values;
    }

    size_t getNumAttributes() const { return _numAttributes; }
    size_t getNumSynapses(const GIDSet& gids) const
    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        size_t numSynapses = 0;
        for (const uint32_t gid : gids)
        {
            Dataset dataset;
            if (!_openDataset(gid, dataset))
                continue;
            numSynapses += dataset.dims.first;
        }
        return numSynapses;
    }

    bool _openDataset(const uint32_t gid, Dataset& dataset) const
    {
        std::stringstream name;
        name << "a" << gid;
        return detail::_openDataset(*_file, name.str(), dataset);
    }

    SynapseMatrix read(const uint32_t gid, const uint32_t attributes) const
    {
        switch (_numAttributes)
        {
        case SYNAPSE_ALL:
            return read<SYNAPSE_ALL>(gid, attributes);
        case SYNAPSE_POSITION_ALL:
            return read<SYNAPSE_POSITION_ALL>(gid, attributes);
        case SYNAPSE_OLD_POSITION_ALL:
            return read<SYNAPSE_OLD_POSITION_ALL>(gid, attributes);
        case 1:
            // nrn_extra
            return read<1>(gid, 1);
        default:
            LBERROR << "Synapse file " << _file->getName()
                    << " has unknown number of attributes: " << _numAttributes
                    << std::endl;
            return SynapseMatrix();
        }
    }

private:
    std::unique_ptr<HighFive::File> _file;
    size_t _numAttributes;
};

/** Implement the logic to read a merged .h5 or individual .h5.<int> files */
class Synapse : public boost::noncopyable
{
public:
    explicit Synapse(const std::string& source)
        : _file(nullptr)
        , _gid(0)
    {
        try
        {
            _file = new SynapseFile(source);
        }
        catch (const std::runtime_error&)
        {
            const fs::path sourcePath(source);
            const fs::path dir = sourcePath.parent_path();
            const std::string filename = sourcePath.filename().generic_string();

            // OPT: check if we have at least one unmerged file before fetching
            // all the filenames. Much faster in case there is no merged and/or
            // unmerged file at all.
            if (!boost::filesystem::exists(fs::path(source + ".0")))
            {
                throw std::runtime_error(
                    "No merged or unmerged synapse file found: " + source);
            }

            LBWARN << "Only unmerged synapse files found for " << source
                   << "; consider using merged files for better performance."
                   << std::endl;

            _findCandidateFiles(dir, filename);
            _createIndex(dir, filename);
        }
    }

    ~Synapse() { delete _file; }
    SynapseMatrix read(const uint32_t gid, const uint32_t attributes) const
    {
        if (_findFile(gid))
            return _file->read(gid, attributes);
        return SynapseMatrix();
    }

    size_t getNumSynapses(const GIDSet& gids) const
    {
        size_t numSynapses = 0;
        for (const uint32_t gid : gids)
        {
            if (!_findFile(gid))
                continue;

            GIDSet set;
            set.insert(gid);
            numSynapses += _file->getNumSynapses(set);
        }
        return numSynapses;
    }

    size_t getNumAttributes() const
    {
        if (_file)
            return _file->getNumAttributes();

        assert(!_fileNames.empty());

        const std::string& filename = _fileNames.front();

        try
        {
            HighFive::SilenceHDF5 silence;
            HighFive::File file(filename, H5F_ACC_RDONLY);
            Dataset dataset;
            if (!_openDataset(file, file.getObjectName(0), dataset))
                LBTHROW(std::runtime_error(
                    "Cannot open dataset in synapse file " + filename));
            return dataset.dims.second;
        }
        catch (const HighFive::FileException& exc)
        {
            LBTHROW(std::runtime_error("Could not open synapse file " +
                                       filename + ": " + exc.what()));
        }
    }

private:
    typedef std::unordered_map<uint32_t, std::string> GidFileMap;

    mutable SynapseFile* _file;
    mutable uint32_t _gid; // current or 0 for all
    Strings _fileNames;
    mutable GidFileMap _fileMap;

    void _createIndex(const fs::path& dir, const std::string& filename)
    {
        // extract the GID->file mapping from the merge_nrn.sh script
        const bool afferent = filename.find("efferent") == std::string::npos;
        const fs::path merge_nrn =
            dir / (afferent ? "merge_nrn.sh" : "merge_nrn_efferent.sh");
        const std::ifstream mergeFile(merge_nrn.generic_string().c_str());
        if (!mergeFile.is_open())
        {
            LBWARN << "No merged file found in " << dir
                   << " to build lookup index; loading data will be very slow"
                   << std::endl;
            return;
        }

        std::stringstream buffer;
        buffer << mergeFile.rdbuf();

        const boost::regex commentregx("#.*?\\n");
        const std::string content =
            boost::regex_replace(buffer.str(), commentregx, "\n");

        const boost::regex regx(
            "\\$CMD -i \\$H5.(?<number>[0-9]+) -o "
            "\\$H5 -s /a(?<gid>[0-9]+)");
        const int subs[] = {1, 2};
        boost::sregex_token_iterator i(content.begin(), content.end(), regx,
                                       subs);

        const std::string basename =
            (dir / fs::path(filename)).generic_string() + ".";
        for (boost::sregex_token_iterator end; i != end;)
        {
            const std::string& fileNumber = *i++;
            const uint32_t fileGID = boost::lexical_cast<uint32_t>(*i++);
            _fileMap[fileGID] = basename + fileNumber;
        }
    }

    void _findCandidateFiles(const fs::path& dir, const std::string& filename)
    {
        // try to open in individual files
        const boost::regex filter(filename + "\\.[0-9]+$");
        fs::directory_iterator end;
        for (fs::directory_iterator i(dir); i != end; ++i)
        {
            const fs::path candidate = i->path().filename();
            boost::smatch match;

            // Testing first if the regex matches. We don't want to throw on
            // is_regular_file for directory entries that are not valid anyway.
            if (boost::regex_match(candidate.string(), match, filter) &&
                boost::filesystem::is_regular_file(i->status()))
            {
                _fileNames.push_back(i->path().string());
            }
        }

        if (_fileNames.empty())
        {
            LBTHROW(std::runtime_error("Could not find synapse files " +
                                       dir.string() + "/" + filename));
        }
    }

    bool _findFile(const uint32_t gid) const
    {
        if (_file && (_gid == gid || _gid == 0))
            return true;

        const std::string& filename = _findFilename(gid);
        if (filename.empty())
            return false;

        delete _file;
        _file = new SynapseFile(filename);
        _gid = gid;
        return true;
    }

    std::string _findFilename(const uint32_t gid) const
    {
        if (!_fileMap[gid].empty())
            return _fileMap[gid];

        // at this point we can only search in each file for the GID which
        // usually results in waiting for I/O and non-parallizable search thanks
        // to HDF5

        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        HighFive::SilenceHDF5 silence;

        // this trial-and-error is the 'fastest' path found
        for (const std::string& candidate : _fileNames)
        {
            try
            {
                HighFive::File file(candidate, H5F_ACC_RDONLY);

                std::stringstream name;
                name << "a" << gid;

                file.getDataSet(name.str());
                _fileMap[gid] = candidate;
                return candidate;
            }
            catch (const HighFive::DataSetException&)
            {
            }
        }
        return std::string();
    }
};
}

Synapse::Synapse(const std::string& source)
    : _impl(new detail::Synapse(source))
{
}

Synapse::~Synapse()
{
    delete _impl;
}

SynapseMatrix Synapse::read(const uint32_t gid, const uint32_t attributes) const
{
    return _impl->read(gid, attributes);
}

size_t Synapse::getNumSynapses(const GIDSet& gids) const
{
    return _impl->getNumSynapses(gids);
}

size_t Synapse::getNumAttributes() const
{
    return _impl->getNumAttributes();
}
}
