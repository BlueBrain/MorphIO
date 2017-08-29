/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include "compartmentReportHDF5.h"
#include "../detail/lockHDF5.h"
#include "../detail/utilsHDF5.h"
#include <brion/version.h>

#include <boost/filesystem.hpp>
#include <boost/scoped_array.hpp>

#include <highfive/util.hpp>

#include <lunchbox/debug.h>
#include <lunchbox/log.h>
#include <lunchbox/pluginRegisterer.h>
#include <lunchbox/scopedMutex.h>

#define CATCH_HDF5ERRORS                             \
    catch (const H5::FileIException& exc)            \
    {                                                \
        LBERROR << exc.getCDetailMsg() << std::endl; \
    }                                                \
    catch (const H5::DataSetIException& exc)         \
    {                                                \
        LBERROR << exc.getCDetailMsg() << std::endl; \
    }                                                \
    catch (const H5::DataSpaceIException& exc)       \
    {                                                \
        LBERROR << exc.getCDetailMsg() << std::endl; \
    }

namespace brion
{
namespace plugin
{
namespace
{
lunchbox::PluginRegisterer<CompartmentReportHDF5> registerer;
}

// Computes compartment counts for a section within a cell inside an ordered
// offset based compartment mapping (see CompartmentReportMapping).
uint16_t calcCompartmentCounts(SectionOffsets::const_iterator cell,
                               uint64_ts::const_iterator section,
                               const SectionOffsets& mapping,
                               const size_t totalCompartments)
{
    const uint64_t firstIndex = *section;
    if (firstIndex == LB_UNDEFINED_UINT64) // Detecting empty sections
        return 0;

    // Skipping sections that don't appear in the report. Right now, this is not
    // very bad since we will only have to skip sections when we ask for the
    // second axon section, due to the remaining axon sections not being
    // reported.
    uint64_t lastIndex = LB_UNDEFINED_UINT64;
    for (++section; section != cell->end() && lastIndex == LB_UNDEFINED_UINT64;
         ++section)
    {
        lastIndex = *section;
    }

    if (lastIndex == LB_UNDEFINED_UINT64)
    {
        // We reached the end of the neuron without finding a section with valid
        // offset. We need to search for the first offset of the next neuron (if
        // any). For simplicity we assume that first section (soma) always has a
        // defined offset.
        while (++cell != mapping.end() && cell->empty())
        {
        }

        if (cell != mapping.end())
        {
            lastIndex = *cell->begin();
            LBASSERT(lastIndex != LB_UNDEFINED_UINT64);
        }
        else
            // No cell with mapping found after the initial one.
            lastIndex = totalCompartments;
    }

    LBASSERT(lastIndex - firstIndex > 0 &&
             lastIndex - firstIndex <= std::numeric_limits<size_t>::max());
    return uint16_t(lastIndex - firstIndex);
}

const std::string mappingDatasetName("mapping");
const std::string dataDatasetName("data");

const std::string mappingAttributes[] = {"type", "sections", "soma",
                                         "axon", "basal",    "apic"};
const std::string dataAttributes[] = {"rank", "tstart", "tstop",
                                      "Dt",   "dunit",  "tunit"};

CompartmentReportHDF5::CompartmentReportHDF5(
    const CompartmentReportInitData& initData)
    : _startTime(0)
    , _endTime(0)
    , _timestep(0)
    , _comps(0)
    , _path(initData.getURI().getPath())
{
    const int accessMode = initData.getAccessMode();

    {
        lunchbox::ScopedWrite mutex(detail::hdf5Lock());
        HighFive::SilenceHDF5 silence;
        namespace fs = boost::filesystem;

        if (accessMode & MODE_WRITE)
        {
            const bool exists = fs::exists(_path);
            if (exists && (accessMode & MODE_OVERWRITE) != MODE_OVERWRITE)
            {
                LBTHROW(std::runtime_error("Cannot overwrite existing file " +
                                           _path.string()));
            }
            const auto writeFlag =
                exists ? HighFive::File::Truncate : HighFive::File::Excl;
            _file.reset(new HighFive::File(_path.string(),
                                           HighFive::File::Create | writeFlag));

            _reportName = fs::basename(_path);
            return;
        }

        // assume one file per cell
        if (fs::is_directory(_path))
        {
            fs::directory_iterator it(_path);
            if (it == fs::directory_iterator())
                LBTHROW(std::runtime_error("No files in " + _path.string()));
            HighFive::File file(it->path().string(), HighFive::File::ReadOnly);
            _readMetaData(file);
        }
        // assume one file for all cells
        else
        {
            _reportName = fs::basename(_path);
            _file.reset(
                new HighFive::File(_path.string(), HighFive::File::ReadOnly));
            _readMetaData(*_file);
        }
    }
    _cacheNeuronCompartmentCounts(initData.getGids());
}

CompartmentReportHDF5::~CompartmentReportHDF5()
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());
    _file.reset();
    _files.clear();
    _datas.clear();
}

bool CompartmentReportHDF5::handles(const CompartmentReportInitData& initData)
{
    const URI& uri = initData.getURI();
    if (!uri.getScheme().empty() && uri.getScheme() != "file")
        return false;

    const boost::filesystem::path ext =
        boost::filesystem::path(uri.getPath()).extension();
    return ext == ".h5" || ext == ".hdf5";
}

std::string CompartmentReportHDF5::getDescription()
{
    return "Blue Brain HDF5 compartment reports:"
           "  [file://]/path/to/report.(h5|hdf5)";
}

const GIDSet& CompartmentReportHDF5::getGIDs() const
{
    return _gids;
}

const SectionOffsets& CompartmentReportHDF5::getOffsets() const
{
    return _offsets;
}

const CompartmentCounts& CompartmentReportHDF5::getCompartmentCounts() const
{
    return _counts;
}

size_t CompartmentReportHDF5::getFrameSize() const
{
    return _comps;
}

bool CompartmentReportHDF5::_loadFrame(const size_t frameNumber,
                                       float* buffer) const
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    size_t cellIndex = 0;
    size_t destOffset = 0;
    for (auto cellID : _gids)
    {
        const auto& dataset = _datas.find(cellID)->second;
        const size_t compartments = getNumCompartments(cellIndex);
        const auto& selection =
            dataset.select({frameNumber, 0}, {1, compartments});

        // Deceiving HighFive into believing this is a two dimensional buffer
        float* ptr = buffer + destOffset;
        selection.read(ptr);

        ++cellIndex;
        destOffset += compartments;
    }
    return true;
}

void CompartmentReportHDF5::updateMapping(const GIDSet& gids)
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    _gids = gids;

    _datas.clear();

    if (_gids.empty())
    {
        if (!_file)
            return;

        for (hsize_t i = 0; i < _file->getNumberObjects(); ++i)
        {
            const std::string& datasetName = _file->getObjectName(i);
            std::stringstream tmp;
            tmp << datasetName.substr(1);
            uint32_t gid;
            tmp >> gid;
            _gids.insert(gid);
        }
    }

    _offsets.resize(_gids.size());

    size_t nextCompartmentIndex = 0;
    size_t cellIndex = 0;
    for (auto cellID : _gids)
    {
        _openFile(cellID);

        const HighFive::File& file =
            _file ? *_file : _files.find(cellID)->second;

        std::stringstream cellName;
        cellName << "a" << cellID;
        const std::string datasetName =
            "/" + cellName.str() + "/" + _reportName + "/" + mappingDatasetName;
        HighFive::DataSet dataset = [file, datasetName]() {
            try
            {
                return file.getDataSet(datasetName);
            }
            catch (HighFive::DataSetException&)
            {
                LBTHROW(std::runtime_error("CompartmentReportHDF5: Dataset " +
                                           datasetName + " not found "
                                                         "in file: " +
                                           file.getName()));
            }
        }();

        auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != 2)
        {
            LBTHROW(
                std::runtime_error("Compartment_Report_HDF5_File_Reader: "
                                   "Error, not 2 dimensional array on " +
                                   datasetName));
        }

        boost::scoped_array<float> buffer(new float[dims[1]]);
        float* ptr = buffer.get(); // HighFive requires an l-value for read
        dataset.read(ptr);

        // Getting the last section id;
        size_t largestSectionID = 0;
        for (size_t i = 0; i < dims[1]; ++i)
        {
            if (buffer[i] > largestSectionID)
                largestSectionID = buffer[i];
        }

        uint64_ts& offsets = _offsets[cellIndex];
        offsets.resize(largestSectionID + 1, LB_UNDEFINED_UINT64);

        size_t lastSection = LB_UNDEFINED_UINT16;
        for (size_t i = 0; i < dims[1]; ++i, ++nextCompartmentIndex)
        {
            const size_t section = buffer[i];
            if (lastSection != section)
            {
                lastSection = section;
                // Storing the start index of a new section with at
                // least 1 compartment
                offsets[section] = nextCompartmentIndex;
            }
        }

        ++cellIndex;
    }

    _comps = nextCompartmentIndex;
    _counts.resize(_offsets.size());

    SectionOffsets::const_iterator offsetsIt;
    CompartmentCounts::iterator countsIt;
    for (offsetsIt = _offsets.begin(), countsIt = _counts.begin();
         offsetsIt != _offsets.end(); ++offsetsIt, ++countsIt)
    {
        countsIt->reserve(offsetsIt->size());

        uint64_ts::const_iterator sectionOffsetsIt;
        for (sectionOffsetsIt = offsetsIt->begin();
             sectionOffsetsIt != offsetsIt->end(); ++sectionOffsetsIt)
        {
            countsIt->push_back(
                calcCompartmentCounts(offsetsIt, sectionOffsetsIt, _offsets,
                                      nextCompartmentIndex));
        }
    }

    for (auto cellID : _gids)
    {
        const auto& file = _file ? *_file : _files.find(cellID)->second;
        auto dataset = _openDataset(file, cellID);
        _datas.emplace(std::make_pair(cellID, std::move(dataset)));
    }
}

void CompartmentReportHDF5::writeHeader(const double startTime,
                                        const double endTime,
                                        const double timestep,
                                        const std::string& dunit,
                                        const std::string& tunit)
{
    LBASSERTINFO(endTime - startTime >= timestep,
                 "Invalid report time " << startTime << ".." << endTime << "/"
                                        << timestep);
    if (timestep <= 0.f)
    {
        std::ostringstream msg;
        msg << "Timestep is not > 0.0, got " << timestep;
        throw std::invalid_argument(msg.str());
    }

    _startTime = startTime;
    _endTime = endTime;
    _timestep = timestep;
    _dunit = dunit;
    _tunit = tunit;

    _createMetaData();
}

bool CompartmentReportHDF5::writeCompartments(const uint32_t gid,
                                              const uint16_ts& counts)
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    try
    {
        const size_t compCount =
            std::accumulate(counts.begin(), counts.end(), 0);
        LBASSERT(!counts.empty());
        LBASSERTINFO(compCount > 0, "No compartments for GID " << gid);
        HighFive::DataSet dataset = _createDataset(gid, compCount);

        const size_t sections = counts.size();
        LBASSERT(sections > 0);
        dataset.getAttribute(mappingAttributes[1]).write(sections);

        boost::multi_array<float, 2> mapping(boost::extents[1][compCount]);
        size_t i = 0;
        for (size_t j = 0; j < counts.size(); ++j)
            for (size_t k = 0; k < counts[j]; ++k)
                mapping[0][i++] = j;

        dataset.write(mapping);
        return true;
    }
    catch (const HighFive::Exception& e)
    {
        LBERROR << "CompartmentReportHDF5: error writing mapping: " << e.what()
                << std::endl;
    }
    return false;
}

bool CompartmentReportHDF5::writeFrame(const uint32_t gid, const float* values,
                                       const size_t /*size*/,
                                       const double timestamp)
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    try
    {
        auto& dataset = _getDataset(gid);
        auto dims = dataset.getSpace().getDimensions();
        const size_t frameNumber = _getFrameNumber(timestamp);
        auto selection = dataset.select({frameNumber, 0}, {1, dims[1]});
        // HighFive is not handling const correctly
        selection.write(const_cast<float*>(values));
        return true;
    }
    catch (const HighFive::Exception& e)
    {
        LBERROR << "CompartmentReportHDF5: error writing frame: " << e.what()
                << std::endl;
    }
    return false;
}

bool CompartmentReportHDF5::flush()
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());
    if (_file)
        _file->flush();
    return true;
}

void CompartmentReportHDF5::_openFile(const uint32_t cellID)
{
    if (_file)
        // All cells are in a single file
        return;

    assert(_files.find(cellID) == _files.end());

    std::stringstream cellName;
    cellName << "a" << cellID;
    const boost::filesystem::path filename = _path / (cellName.str() + ".h5");

    try
    {
        HighFive::SilenceHDF5 silence;
        _files.emplace(
            std::make_pair(cellID, HighFive::File(filename.string().c_str(),
                                                  H5F_ACC_RDONLY)));
    }
    catch (const HighFive::FileException&)
    {
        LBTHROW(std::runtime_error(
            "CompartmentReportHDF5: error opening file:" + filename.string()));
    }
}

HighFive::DataSet CompartmentReportHDF5::_openDataset(
    const HighFive::File& file, const uint32_t cellID)
{
    std::stringstream cellName;
    cellName << "a" << cellID;
    const std::string datasetName =
        "/" + cellName.str() + "/" + _reportName + "/" + dataDatasetName;
    HighFive::DataSet dataset = [file, datasetName]() {
        try
        {
            return file.getDataSet(datasetName);
        }
        catch (const HighFive::DataSetException&)
        {
            LBTHROW(std::runtime_error("CompartmentReportHDF5: Dataset " +
                                       datasetName + " not found in file: " +
                                       file.getName()));
        }
    }();

    if (dataset.getSpace().getNumberDimensions() != 2)
    {
        LBTHROW(
            std::runtime_error("CompartmentReportHDF5: "
                               "Error, not 2 dimensional array on " +
                               datasetName));
    }

    return dataset;
}

HighFive::DataSet CompartmentReportHDF5::_createDataset(const uint32_t gid,
                                                        const size_t compCount)
{
    LBASSERT(compCount > 0);
    LBASSERT(!_reportName.empty());

    std::ostringstream neuronName;
    neuronName << "a" << gid;

    HighFive::Group neuronGroup = _file->createGroup(neuronName.str().c_str());
    HighFive::Group reportGroup = neuronGroup.createGroup(_reportName);

    const double step = getTimestep();
    // Adding step / 2 to the window to avoid off by 1 errors during truncation
    // after the division.
    const size_t numSteps = (getEndTime() - getStartTime() + step * 0.5) / step;
    LBASSERT(numSteps > 0);

    HighFive::DataSet mappingDataset =
        reportGroup.createDataSet<float>(mappingDatasetName,
                                         HighFive::DataSpace({1, compCount}));
    HighFive::DataSet dataDataset = reportGroup.createDataSet<float>(
        dataDatasetName, HighFive::DataSpace({numSteps, compCount}));

    _datas.emplace(std::make_pair(gid, std::move(dataDataset)));

    _createMappingAttributes(mappingDataset);
    _createDataAttributes(dataDataset);

    return mappingDataset;
}

HighFive::DataSet& CompartmentReportHDF5::_getDataset(const uint32_t gid)
{
    Datasets::iterator it = _datas.find(gid);
    if (it == _datas.end())
        throw std::runtime_error("Need mapping before writing data");
    return it->second;
}

void CompartmentReportHDF5::_readMetaData(const HighFive::File& file)
{
    if (!file.getNumberObjects())
    {
        std::runtime_error exc("File is empty: " + file.getName());
        LBTHROW(exc);
    }

    try
    {
        HighFive::SilenceHDF5 silence;
        const std::string& datasetName = file.getObjectName(0);
        const auto& reportGroup = file.getGroup(datasetName);
        _reportName = reportGroup.getObjectName(0);
        std::stringstream tmp;
        tmp << datasetName.substr(1);
        uint32_t gid;
        tmp >> gid;

        auto dataset = _openDataset(*_file, gid);
        dataset.getAttribute(dataAttributes[1]).read(_startTime);
        dataset.getAttribute(dataAttributes[2]).read(_endTime);
        dataset.getAttribute(dataAttributes[3]).read(_timestep);
        _dunit = "mV";
        _tunit = "ms";
    }
    catch (const HighFive::Exception&)
    {
        LBTHROW(std::runtime_error(_path.string() +
                                   " not a valid H5 compartment report file"));
    }
}

void CompartmentReportHDF5::_createMetaData()
{
    auto root = _file->getGroup("/");

    detail::addStringAttribute(root, "creator", "Brion");
    detail::addStringAttribute(root, "software_version", BRION_REV_STRING);

    const time_t now = ::time(0);
#ifdef _WIN32
    char* gmtString = ::ctime(&now);
#else
    char gmtString[32];
    ::ctime_r(&now, gmtString);
#endif
    std::string creationTimeName = gmtString;
    // ctime_r ends with \n
    creationTimeName = creationTimeName.substr(0, creationTimeName.size() - 1);
    detail::addStringAttribute(root, "creation_time", creationTimeName);
}

void CompartmentReportHDF5::_createMappingAttributes(HighFive::DataSet& dataset)
{
    const std::string type = "compartment";
    detail::addStringAttribute(dataset, mappingAttributes[0], type);
    for (int i = 1; i < 6; ++i)
        dataset.createAttribute<int>(mappingAttributes[i],
                                     HighFive::DataSpace(
                                         std::vector<size_t>({1})));
}

void CompartmentReportHDF5::_createDataAttributes(HighFive::DataSet& dataset)
{
    HighFive::DataSpace scalar(std::vector<size_t>({1}));

    auto attribute =
        dataset.createAttribute<int32_t>(dataAttributes[0], scalar);
    attribute.write(0); // rank

    attribute = dataset.createAttribute<double>(dataAttributes[1], scalar);
    attribute.write(getStartTime());

    attribute = dataset.createAttribute<double>(dataAttributes[2], scalar);
    attribute.write(getEndTime());

    attribute = dataset.createAttribute<double>(dataAttributes[3], scalar);
    attribute.write(getTimestep());

    detail::addStringAttribute(dataset, dataAttributes[4], _dunit);
    detail::addStringAttribute(dataset, dataAttributes[5], _tunit);
}
}
}
