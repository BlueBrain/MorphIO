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
#include "../detail/silenceHDF5.h"
#include "../detail/utilsHDF5.h"
#include <brion/version.h>

#include <boost/filesystem.hpp>
#include <boost/scoped_array.hpp>
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

        namespace fs = boost::filesystem;

        if (accessMode & MODE_WRITE)
        {
            if ((accessMode & MODE_OVERWRITE) != MODE_OVERWRITE &&
                fs::exists(_path))
            {
                LBTHROW(std::runtime_error("Cannot overwrite existing file " +
                                           _path.string()));
            }

            _file = H5::H5File(_path.string(), H5F_ACC_TRUNC);
            _reportName = fs::basename(_path);
            return;
        }

        H5::H5File file;

        // assume one file per cell
        if (fs::is_directory(_path))
        {
            fs::directory_iterator it(_path);
            if (it == fs::directory_iterator())
                LBTHROW(std::runtime_error("No files in " + _path.string()));
            file = H5::H5File(it->path().string(), H5F_ACC_RDONLY);
            if (!file.getId())
                LBTHROW(std::runtime_error("File not found: " +
                                           it->path().string()));
        }
        // assume one file for all cells
        else
        {
            _reportName = fs::basename(_path);
            _file = H5::H5File(_path.string(), H5F_ACC_RDONLY);
            if (!_file.getId())
                LBTHROW(
                    std::runtime_error("File not found: " + _path.string()));
            file = _file;
        }

        if (!file.getNumObjs())
            LBTHROW(std::runtime_error("File is empty: " + file.getFileName()));

        try
        {
            detail::SilenceHDF5 silence;
            const H5std_string& datasetName = file.getObjnameByIdx(0);
            const H5::Group& reportGroup = file.openGroup(datasetName);
            _reportName = reportGroup.getObjnameByIdx(0);
            std::stringstream tmp;
            tmp << datasetName.substr(1);
            uint32_t gid;
            tmp >> gid;

            const H5::DataSet& dataset = _openDataset(file, gid);
            dataset.openAttribute(dataAttributes[1])
                .read(H5::PredType::NATIVE_DOUBLE, &_startTime);
            dataset.openAttribute(dataAttributes[2])
                .read(H5::PredType::NATIVE_DOUBLE, &_endTime);
            dataset.openAttribute(dataAttributes[3])
                .read(H5::PredType::NATIVE_DOUBLE, &_timestep);
            _dunit = "mV";
            _tunit = "ms";
        }
        catch (...)
        {
            LBTHROW(std::runtime_error(_path.string() +
                                       " not a valid H5"
                                       " compartment report file"));
        }
    }
    _cacheNeuronCompartmentCounts(initData.getGids());
}

CompartmentReportHDF5::~CompartmentReportHDF5()
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    if (_file.getId())
        _file.close();
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

    // The offset for the first comparment of the cell being processed
    hsize_t firstCompartmentOffset = 0;

    for (GIDSetCIter cellID = _gids.begin(); cellID != _gids.end(); ++cellID)
    {
        const H5::DataSet& dataset = _datas.find(*cellID)->second;

        const H5::DataSpace& space = _fspaces.find(*cellID)->second;
        const H5::DataSpace& targetSpace = _mspaces.find(*cellID)->second;

        hsize_t sourceSizes[2];
        space.getSimpleExtentDims(sourceSizes);
        const hsize_t readCounts[2] = {1, sourceSizes[1]};

        const hsize_t readOffsets[2] = {frameNumber, 0};
        space.selectHyperslab(H5S_SELECT_SET, readCounts, readOffsets);

        const hsize_t targetOffsets[2] = {0, firstCompartmentOffset};
        targetSpace.selectHyperslab(H5S_SELECT_SET, readCounts, targetOffsets);

        dataset.read(buffer, H5::PredType::NATIVE_FLOAT, targetSpace, space);

        firstCompartmentOffset += sourceSizes[1];
    }
    return true;
}

void CompartmentReportHDF5::updateMapping(const GIDSet& gids)
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    _gids = gids;

    _datas.clear();
    _fspaces.clear();
    _mspaces.clear();

    if (_gids.empty())
    {
        if (!_file.getId())
            return;

        for (hsize_t i = 0; i < _file.getNumObjs(); ++i)
        {
            const H5std_string& datasetName = _file.getObjnameByIdx(i);
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
    for (GIDSetCIter cellID = _gids.begin(); cellID != _gids.end();
         ++cellID, ++cellIndex)
    {
        _openFile(*cellID);

        const H5::H5File& file =
            _file.getId() ? _file : _files.find(*cellID)->second;

        std::stringstream cellName;
        cellName << "a" << *cellID;
        const std::string datasetName =
            "/" + cellName.str() + "/" + _reportName + "/" + mappingDatasetName;
        H5::DataSet dataset;
        H5E_BEGIN_TRY
        dataset = file.openDataSet(datasetName);
        H5E_END_TRY
        if (!dataset.getId())
        {
            LBTHROW(
                std::runtime_error("ReportReaderHDF5: "
                                   "Dataset " +
                                   datasetName + " not found "
                                                 "in file: " +
                                   file.getFileName()));
        }

        if (dataset.getSpace().getSimpleExtentNdims() != 2)
        {
            LBTHROW(
                std::runtime_error("Compartment_Report_HDF5_File_Reader: "
                                   "Error, not 2 dimensional array on " +
                                   datasetName));
        }

        hsize_t dims[2];
        dataset.getSpace().getSimpleExtentDims(dims);

        boost::scoped_array<float> buffer(new float[dims[1]]);
        dataset.read(buffer.get(), H5::PredType::NATIVE_FLOAT);

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

    for (GIDSetCIter cellID = _gids.begin(); cellID != _gids.end();
         ++cellID, ++cellIndex)
    {
        const H5::H5File& file =
            _file.getId() ? _file : _files.find(*cellID)->second;
        const H5::DataSet& dataset = _openDataset(file, *cellID);
        _datas[*cellID] = dataset;
        _fspaces[*cellID] = dataset.getSpace();
        hsize_t targetSizes[2] = {1, getFrameSize()};
        H5::DataSpace targetSpace(2, targetSizes);
        _mspaces[*cellID] = targetSpace;
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
        H5::DataSet dataset = _createDataset(gid, compCount);

        const size_t sections = counts.size();
        LBASSERT(sections > 0);
        dataset.openAttribute(1).write(H5::PredType::NATIVE_INT, &sections);

        boost::scoped_array<float> mapping(new float[compCount]);
        size_t i = 0;
        for (size_t j = 0; j < counts.size(); ++j)
            for (size_t k = 0; k < counts[j]; ++k)
                mapping[i++] = j;

        dataset.write(mapping.get(), H5::PredType::NATIVE_FLOAT);
        return true;
    }
    CATCH_HDF5ERRORS
    return false;
}

bool CompartmentReportHDF5::writeFrame(const uint32_t gid, const float* values,
                                       const size_t /*size*/,
                                       const double timestamp)
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());

    try
    {
        H5::DataSet& dataset = _getDataset(gid);
        const H5::DataSpace& dataFspace = dataset.getSpace();

        const size_t frameNumber = _getFrameNumber(timestamp);

        hsize_t dims[2];
        dataFspace.getSimpleExtentDims(dims);

        // get the correct slab
        const hsize_t dataDim[2] = {1, dims[1]};
        const hsize_t dataOffset[2] = {frameNumber, 0};
        dataFspace.selectHyperslab(H5S_SELECT_SET, dataDim, dataOffset);

        // define mspace
        H5::DataSpace dataMspace(1, &dataDim[1]);

        // Write data to the dataset
        dataset.write(values, H5::PredType::NATIVE_FLOAT, dataMspace,
                      dataFspace);
        return true;
    }
    CATCH_HDF5ERRORS
    return false;
}

bool CompartmentReportHDF5::flush()
{
    lunchbox::ScopedWrite mutex(detail::hdf5Lock());
    _file.flush(H5F_SCOPE_GLOBAL);
    return true;
}

void CompartmentReportHDF5::_openFile(const uint32_t cellID)
{
    if (_file.getId())
        return;

    assert(_files.find(cellID) == _files.end());

    std::stringstream cellName;
    cellName << "a" << cellID;
    const boost::filesystem::path filename = _path / (cellName.str() + ".h5");

    H5::H5File file(filename.string().c_str(), H5F_ACC_RDONLY);
    if (!file.getId())
    {
        LBTHROW(std::runtime_error("ReportReaderHDF5: error opening file:" +
                                   filename.string()));
    }
    _files[cellID] = file;
}

H5::DataSet CompartmentReportHDF5::_openDataset(const H5::H5File& file,
                                                const uint32_t cellID)
{
    std::stringstream cellName;
    cellName << "a" << cellID;
    const std::string datasetName =
        "/" + cellName.str() + "/" + _reportName + "/" + dataDatasetName;
    H5::DataSet dataset;
    H5E_BEGIN_TRY
    dataset = file.openDataSet(datasetName);
    H5E_END_TRY
    if (!dataset.getId())
    {
        LBTHROW(std::runtime_error("ReportReaderHDF5: Dataset " + datasetName +
                                   " not found in file: " +
                                   file.getFileName()));
    }

    if (dataset.getSpace().getSimpleExtentNdims() != 2)
    {
        LBTHROW(
            std::runtime_error("Compartment_Report_HDF5_File_Reader: "
                               "Error, not 2 dimensional array on " +
                               datasetName));
    }

    return dataset;
}

H5::DataSet CompartmentReportHDF5::_createDataset(const uint32_t gid,
                                                  const size_t compCount)
{
    LBASSERT(compCount > 0);
    LBASSERT(!_reportName.empty());

    std::ostringstream neuronName;
    neuronName << "a" << gid;

    H5::Group neuronGroup = _file.createGroup(neuronName.str().c_str());
    H5::Group reportGroup = neuronGroup.createGroup(_reportName);

    const int dims = 2;
    const double step = getTimestep();
    // Adding step / 2 to the window to avoid off by 1 errors during truncation
    // after the division.
    const size_t numSteps = (getEndTime() - getStartTime() + step * 0.5) / step;
    const hsize_t mappingDim[dims] = {1, compCount};
    const hsize_t dataDim[dims] = {numSteps, compCount};
    LBASSERT(numSteps > 0);

    H5::DataSpace mappingDataspace(dims, mappingDim);
    H5::DataSpace dataDataspace(dims, dataDim);

    H5::DataSet mappingDataset =
        reportGroup.createDataSet(mappingDatasetName,
                                  H5::PredType::NATIVE_FLOAT, mappingDataspace);
    H5::DataSet dataDataset =
        reportGroup.createDataSet(dataDatasetName, H5::PredType::NATIVE_FLOAT,
                                  dataDataspace);

    _datas[gid] = dataDataset;

    _createMappingAttributes(mappingDataset);
    _createDataAttributes(dataDataset);

    return mappingDataset;
}

H5::DataSet& CompartmentReportHDF5::_getDataset(const uint32_t gid)
{
    Datasets::iterator it = _datas.find(gid);
    if (it == _datas.end())
        throw std::runtime_error("Need mapping before writing data");
    return it->second;
}

void CompartmentReportHDF5::_createMetaData()
{
    H5::Group root = _file.openGroup("/");

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

void CompartmentReportHDF5::_createMappingAttributes(H5::DataSet& dataset)
{
    const std::string type = "compartment";
    detail::addStringAttribute(dataset, mappingAttributes[0], type);
    dataset.createAttribute(mappingAttributes[1], H5::PredType::NATIVE_INT,
                            H5S_SCALAR);
    dataset.createAttribute(mappingAttributes[2], H5::PredType::NATIVE_INT,
                            H5S_SCALAR);
    dataset.createAttribute(mappingAttributes[3], H5::PredType::NATIVE_INT,
                            H5S_SCALAR);
    dataset.createAttribute(mappingAttributes[4], H5::PredType::NATIVE_INT,
                            H5S_SCALAR);
    dataset.createAttribute(mappingAttributes[5], H5::PredType::NATIVE_INT,
                            H5S_SCALAR);
}

void CompartmentReportHDF5::_createDataAttributes(H5::DataSet& dataset)
{
    const int rank = 0;
    const double startTime = getStartTime();
    const double endTime = getEndTime();
    const double timestep = getTimestep();

    H5::Attribute rankAttr =
        dataset.createAttribute(dataAttributes[0], H5::PredType::NATIVE_INT,
                                H5S_SCALAR);
    H5::Attribute tstartAttr =
        dataset.createAttribute(dataAttributes[1], H5::PredType::NATIVE_DOUBLE,
                                H5S_SCALAR);
    H5::Attribute tstopAttr =
        dataset.createAttribute(dataAttributes[2], H5::PredType::NATIVE_DOUBLE,
                                H5S_SCALAR);
    H5::Attribute dtAttr =
        dataset.createAttribute(dataAttributes[3], H5::PredType::NATIVE_DOUBLE,
                                H5S_SCALAR);

    rankAttr.write(H5::PredType::NATIVE_INT, &rank);
    tstartAttr.write(H5::PredType::NATIVE_DOUBLE, &startTime);
    tstopAttr.write(H5::PredType::NATIVE_DOUBLE, &endTime);
    dtAttr.write(H5::PredType::NATIVE_DOUBLE, &timestep);
    detail::addStringAttribute(dataset, dataAttributes[4], _dunit);
    detail::addStringAttribute(dataset, dataAttributes[5], _tunit);
}
}
}
