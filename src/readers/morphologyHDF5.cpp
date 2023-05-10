/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of MorphIO <https://github.com/BlueBrain/MorphIO>
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

#include <cassert>

#include "morphologyHDF5.h"

#include <highfive/H5Utility.hpp>  // HighFive::SilenceHDF5

#include <morphio/errorMessages.h>

namespace {

constexpr size_t SECTION_START_OFFSET = 0;
constexpr size_t SECTION_TYPE = 1;
constexpr size_t SECTION_PARENT_OFFSET = 2;

constexpr int SOMA_ONLY = -1;

//{v1
const std::string _d_structure("structure");
const std::string _d_points("points");

//{ v1.1
const std::string _a_version("version");
const std::string _g_metadata("metadata");
const std::string _a_family("cell_family");
const std::string _d_perimeters("perimeters");
//} v1.1

//{ v1.2
const std::string _g_mitochondria("organelles/mitochondria");

// endoplasmic reticulum
const std::string _g_endoplasmic_reticulum("organelles/endoplasmic_reticulum");
const std::string _d_section_index("section_index");
const std::string _d_volume("volume");
const std::string _d_surface_area("surface_area");
const std::string _d_filament_count("filament_count");
// } v1.2

//{ v1.3

// Dendritic Spine
const std::string _g_postsynaptic_density("organelles/postsynaptic_density");
const std::string _d_dendritic_spine_section_id("section_id");
const std::string _d_dendritic_spine_segment_id("segment_id");
const std::string _d_dendritic_spine_offset("offset");

//}

// } v1

//{ v2
const std::string _g_v2root("neuron1");
//} v2

}  // namespace

namespace morphio {
namespace readers {
namespace h5 {


namespace detail {

template <class Derived>
template <typename T>
void DataSetReader<Derived>::read(const std::string& groupName,
                                  const std::string& datasetName,
                                  std::vector<size_t> expectedDimensions,
                                  T& data) const {
    auto derived = static_cast<const Derived&>(*this);
    derived.read_impl(groupName, datasetName, expectedDimensions, data);
}

template <typename T>
void MergedReader::read_impl(const std::string& groupName,
                             const std::string& datasetName,
                             std::vector<size_t> expectedDimensions,
                             T& data) const {
    if (groupName != "" && !_group.exist(groupName)) {
        throw RawDataError("Missing required group " + groupName);
    }
    const auto group = groupName == "" ? _group : _group.getGroup(groupName);

    if (!group.exist(datasetName)) {
        throw RawDataError("Missing required dataset " + datasetName);
    }
    const HighFive::DataSet dataset = group.getDataSet(datasetName);
    const HighFive::DataSpace dataspace = dataset.getSpace();

    const auto dims = dataspace.getDimensions();
    if (dims.size() != expectedDimensions.size()) {
        throw RawDataError("bad number of dimensions in " + datasetName);
    }

    for (size_t k = 0; k < dims.size(); ++k) {
        if (expectedDimensions[k] != size_t(-1) && expectedDimensions[k] != dims[k]) {
            throw RawDataError("dimension mismatch, dims[" + std::to_string(k) +
                               "] == " + std::to_string(dims[k]) +
                               " != " + std::to_string(expectedDimensions[k]));
        }
    }

    if (dataspace.getElementCount() > 0) {
        // Guards against a bug in HighFive (fixed in 2.7.0) related to
        // empty datasets.
        dataset.read(data);
    } else {
        // If the dataset is empty, the vector should be too. In all other
        // cases HighFive will allocate as appropriate.
        data.resize(0);
    }
}


template <typename T>
void UnifiedReader::read_impl(const std::string& groupName,
                              const std::string& datasetName,
                              std::vector<size_t> expectedDimensions,
                              T& data) const {
    auto make_hyperslab = [](const std::pair<size_t, size_t>& range,
                             const std::vector<size_t>& global_dims) {
        auto rank = global_dims.size();
        if (rank == 0) {
            throw std::invalid_argument("'global_dims' must not be empty.");
        }

        auto begin = range.first;
        auto end = range.second;

        auto h5_offset = std::vector<size_t>(rank, 0ul);
        h5_offset[0] = begin;

        auto h5_count = std::vector<size_t>(rank, 1ul);
        auto h5_stride = std::vector<size_t>(rank, 1ul);

        auto h5_block = global_dims;
        h5_block[0] = end - begin;

        return HighFive::RegularHyperSlab(h5_offset, h5_count, h5_stride, h5_block);
    };

    auto fetch_dataset = [this](const std::string& unifiedName) -> const HighFive::DataSet& {
        auto it = _datasets.find(unifiedName);
        if (it != _datasets.end()) {
            return it->second;
        } else {
            return _datasets[unifiedName] = _group.getFile().getDataSet(".raw/" + unifiedName +
                                                                        "/values");
        }
    };

    auto fetch_global_dims =
        [this, fetch_dataset](const std::string& unifiedName) -> const std::vector<size_t>& {
        auto it = _global_dims.find(unifiedName);
        if (it != _global_dims.end()) {
            return it->second;
        } else {
            auto ds = fetch_dataset(unifiedName);
            return _global_dims[unifiedName] = ds.getDimensions();
        }
    };

    auto unifiedName = groupName + (groupName == "" ? "" : "/") + datasetName;

    auto range =
        std::pair<size_t, size_t>{_group.getAttribute(unifiedName + "_begin").read<size_t>(),
                                  _group.getAttribute(unifiedName + "_end").read<size_t>()};

    auto& dataset = fetch_dataset(unifiedName);
    auto hyperslab = make_hyperslab(range, fetch_global_dims(unifiedName));
    auto memspace = HighFive::DataSpace(hyperslab.block);

    dataset.select(HighFive::HyperSlab(hyperslab), memspace).read(data);
}

}  // namespace detail


MorphologyHDF5::MorphologyHDF5(const HighFive::Group& group)
    : _group(group)
    , _uri("HDF5 Group")
    , _file_version(-1)
    , _merged_reader(_group)
    , _unified_reader(_group) {
    auto file = _group.getFile();
    auto version_exists = H5Aexists(file.getId(), "version");
    if (version_exists < 0) {
        throw RawDataError("H5Aexists failed");
    }

    if (version_exists > 0) {
        _file_version = file.getAttribute("version").read<int>();
    }
}

Property::Properties load(const std::string& uri) {
    try {
        std::lock_guard<std::recursive_mutex> lock(morphio::readers::h5::global_hdf5_mutex());
        HighFive::SilenceHDF5 silence;
        auto file = HighFive::File(uri, HighFive::File::ReadOnly);
        return MorphologyHDF5(file.getGroup("/")).load();

    } catch (const HighFive::FileException& exc) {
        throw RawDataError("Could not open morphology file " + uri + ": " + exc.what());
    }
}

Property::Properties load(const HighFive::Group& group) {
    std::lock_guard<std::recursive_mutex> lock(morphio::readers::h5::global_hdf5_mutex());
    return MorphologyHDF5(group).load();
}

Property::Properties MorphologyHDF5::load() {
    _readMetadata(_uri);

    int firstSectionOffset = _readSections();

    _readPoints(firstSectionOffset);

    if (_properties._cellLevel.minorVersion() >= 1) {
        _readPerimeters(firstSectionOffset);

        if (_properties._cellLevel.minorVersion() >= 2) {
            _readMitochondria();
            _readEndoplasmicReticulum();
        }

        if (_properties._cellLevel.minorVersion() >= 3 &&
            _properties._cellLevel._cellFamily == CellFamily::SPINE) {
            _readDendriticSpinePostSynapticDensity();
        }
    }

    return _properties;
}

void MorphologyHDF5::_readMetadata(const std::string& source) {
    // default to h5v1.0
    uint32_t majorVersion = 1;
    uint32_t minorVersion = 0;
    _properties._cellLevel._cellFamily = CellFamily::NEURON;

    if (!_group.exist(_d_points) || !_group.exist(_d_structure)) {
        // h5v2 is deprecated, but it can be detected, throw a custom error messages if it is
        if (_group.exist(_g_v2root)) {
            throw RawDataError(
                "Error in " + source +
                "\nh5v2 is no longer supported, see: https://github.com/BlueBrain/MorphIO#H5v2");
        }
        throw RawDataError("Missing " + _d_points + " or " + _d_structure +
                           " datasets, cannot load morphology without them");
    }

    // if there is metadata, perhaps it's h5v1, minor version 1, 2
    if (_group.exist(_g_metadata)) {
        const auto metadata = _group.getGroup(_g_metadata);
        if (metadata.hasAttribute(_a_version)) {
            const auto attr = metadata.getAttribute(_a_version);

            std::array<uint32_t, 2> versions = {0, 0};
            attr.read(versions);
            majorVersion = versions[0];
            minorVersion = versions[1];

            if (majorVersion == 1 &&
                (minorVersion == 1 || minorVersion == 2 || minorVersion == 3)) {
                uint32_t family;
                metadata.getAttribute(_a_family).read(family);
                _properties._cellLevel._cellFamily = static_cast<CellFamily>(family);
            } else {
                std::string msg = "Error in " + source +
                                  "\nUnsupported h5 version: " + std::to_string(majorVersion) +
                                  "." + std::to_string(minorVersion);
                throw RawDataError(msg);
            }
        } else {
            throw RawDataError("Missing " + _a_version +
                               " attribute, cannot load morphology without them");
        }
    }

    _properties._cellLevel._version = {"h5", majorVersion, minorVersion};
}

void MorphologyHDF5::_readPoints(int firstSectionOffset) {
    constexpr size_t pointColumns = 4;

    std::vector<std::array<floatType, pointColumns>> hdf5Data;
    _read("", _d_points, std::vector<size_t>{size_t(-1), pointColumns}, hdf5Data);

    const size_t numberPoints = hdf5Data.size();

    const bool hasSoma = firstSectionOffset != 0;
    const bool hasNeurites = static_cast<size_t>(firstSectionOffset) < numberPoints;
    const size_t somaPointCount = hasNeurites ? static_cast<size_t>(firstSectionOffset)
                                              : hdf5Data.size();

    auto& somaPoints = _properties._somaLevel._points;
    auto& somaDiameters = _properties._somaLevel._diameters;

    if (hasSoma) {
        somaPoints.resize(somaPointCount);
        somaDiameters.resize(somaPointCount);

        for (size_t i = 0; i < somaPointCount; ++i) {
            const auto& p = hdf5Data[i];
            somaPoints[i] = {p[0], p[1], p[2]};
            somaDiameters[i] = p[3];
        }
    }

    auto& points = _properties.get_mut<Property::Point>();
    auto& diameters = _properties.get_mut<Property::Diameter>();

    if (hasNeurites) {
        const size_t size = (hdf5Data.size() - somaPointCount);
        points.resize(size);
        diameters.resize(size);
        for (size_t i = somaPointCount; i < hdf5Data.size(); ++i) {
            const auto& p = hdf5Data[i];
            const size_t section = i - somaPointCount;
            points[section] = {p[0], p[1], p[2]};
            diameters[section] = p[3];
        }
    }
}

int MorphologyHDF5::_readSections() {
    // Important: The code used to split the reading of the sections and types
    //            into two separate fine-grained H5 selections. This does not
    //            reduce the number of I/O operations, but increases them by
    //            forcing HDF5 + MPI-IO to read in 4-byte groups. Thus, we now
    //            read the whole dataset at once, and split it in memory.

    constexpr size_t structureV1Columns = 3;

    std::vector<std::array<int, structureV1Columns>> vec;
    _read("", _d_structure, std::vector<size_t>{size_t(-1), structureV1Columns}, vec);

    assert(!vec.empty());

    bool hasSoma = true;
    if (static_cast<SectionType>(vec[0][SECTION_TYPE]) != SECTION_SOMA) {
        hasSoma = false;
    } else if (vec.size() == 1) {
        return SOMA_ONLY;
    }

    const size_t firstSection = hasSoma ? 1 : 0;
    const int firstSectionOffset = vec[firstSection][SECTION_START_OFFSET];

    auto& sections = _properties.get_mut<Property::Section>();
    sections.reserve(vec.size() - firstSection);

    auto& types = _properties.get_mut<Property::SectionType>();
    types.reserve(vec.size() - firstSection);

    // The first section is skipped if it corresponds to a soma
    for (size_t i = firstSection; i < vec.size(); ++i) {
        const auto& section = vec[i];
        SectionType type = static_cast<SectionType>(section[SECTION_TYPE]);

        if (section[SECTION_TYPE] >= SECTION_OUT_OF_RANGE_START || section[SECTION_TYPE] <= 0) {
            ErrorMessages err;
            throw RawDataError(err.ERROR_UNSUPPORTED_SECTION_TYPE(0, type));
        } else if (!hasSoma && type == SECTION_SOMA) {
            throw RawDataError("Error reading morphology " + _uri +
                               ": it has soma section that doesn't come first");
        } else if (hasSoma && type == SECTION_SOMA) {
            throw RawDataError("Error reading morphology " + _uri +
                               ": it has multiple soma sections");
        }

        sections.emplace_back(
            Property::Section::Type{section[SECTION_START_OFFSET] - firstSectionOffset,
                                    section[SECTION_PARENT_OFFSET] - (hasSoma ? 1 : 0)});
        types.emplace_back(type);
    }

    return firstSectionOffset;
}

void MorphologyHDF5::_readPerimeters(int firstSectionOffset) {
    assert(_properties._cellLevel.majorVersion() == 1 &&
           _properties._cellLevel.minorVersion() > 0 &&
           "Perimeter information is available starting at v1.1");

    // soma only, won't have perimeters
    if (firstSectionOffset == SOMA_ONLY) {
        return;
    }

    if (!_group.exist(_d_perimeters)) {
        if (_properties._cellLevel._cellFamily == GLIA) {
            throw RawDataError("No empty perimeters allowed for glia morphology");
        }
        return;
    }

    auto& perimeters = _properties.get_mut<Property::Perimeter>();
    _read("", _d_perimeters, 1, perimeters);
    perimeters.erase(perimeters.begin(), perimeters.begin() + firstSectionOffset);
}

template <typename T>
void MorphologyHDF5::_read(const std::string& groupName,
                           const std::string& datasetName,
                           unsigned int expectedDimension,
                           T& data) {
    auto expectedDimensions = std::vector<size_t>(expectedDimension, size_t(-1));
    _read(groupName, datasetName, expectedDimensions, data);
}

template <typename T>
void MorphologyHDF5::_read(const std::string& groupName,
                           const std::string& datasetName,
                           const std::vector<size_t>& expectedDimensions,
                           T& data) {
    try {
        if (_file_version <= 3) {
            _merged_reader.read(groupName, datasetName, expectedDimensions, data);
        } else {
            _unified_reader.read(groupName, datasetName, expectedDimensions, data);
        }
    } catch (const RawDataError& err) {
        throw RawDataError("Reading morphology '" + _uri + "': " + err.what());
    }
}

void MorphologyHDF5::_readDendriticSpinePostSynapticDensity() {
    std::vector<morphio::Property::DendriticSpine::SectionId_t> sectionIds;
    _read(_g_postsynaptic_density, _d_dendritic_spine_section_id, 1, sectionIds);

    std::vector<morphio::Property::DendriticSpine::SegmentId_t> segmentIds;
    _read(_g_postsynaptic_density, _d_dendritic_spine_segment_id, 1, segmentIds);

    std::vector<morphio::Property::DendriticSpine::Offset_t> offsets;
    _read(_g_postsynaptic_density, _d_dendritic_spine_offset, 1, offsets);

    if (sectionIds.size() != segmentIds.size() || offsets.size() != segmentIds.size()) {
        throw RawDataError(
            "Dendritic datasets must match in size:"
            " sectionIds: " +
            std::to_string(sectionIds.size()) + " segmentIds: " +
            std::to_string(segmentIds.size()) + " offsets: " + std::to_string(offsets.size()));
    }

    auto& properties = _properties._dendriticSpineLevel._post_synaptic_density;

    properties.reserve(sectionIds.size());
    for (size_t i = 0; i < sectionIds.size(); ++i) {
        properties.push_back({sectionIds[i], segmentIds[i], offsets[i]});
    }
}

void MorphologyHDF5::_readEndoplasmicReticulum() {
    if (!_group.exist(_g_endoplasmic_reticulum)) {
        return;
    }

    _read(_g_endoplasmic_reticulum,
          _d_section_index,
          1,
          _properties._endoplasmicReticulumLevel._sectionIndices);
    _read(_g_endoplasmic_reticulum,
          _d_volume,
          1,
          _properties._endoplasmicReticulumLevel._volumes);
    _read(_g_endoplasmic_reticulum,
          _d_surface_area,
          1,
          _properties._endoplasmicReticulumLevel._surfaceAreas);
    _read(_g_endoplasmic_reticulum,
          _d_filament_count,
          1,
          _properties._endoplasmicReticulumLevel._filamentCounts);
}

void MorphologyHDF5::_readMitochondria() {
    if (!_group.exist(_g_mitochondria)) {
        return;
    }

    std::vector<std::vector<floatType>> points;
    _read(_g_mitochondria, _d_points, 2, points);

    auto& mitoSectionId = _properties.get_mut<Property::MitoNeuriteSectionId>();
    auto& pathlength = _properties.get_mut<Property::MitoPathLength>();
    auto& diameters = _properties.get_mut<Property::MitoDiameter>();
    mitoSectionId.reserve(mitoSectionId.size() + points.size());
    pathlength.reserve(pathlength.size() + points.size());
    diameters.reserve(diameters.size() + points.size());
    for (const auto& p : points) {
        mitoSectionId.push_back(static_cast<Property::MitoNeuriteSectionId::Type>(p[0]));
        pathlength.push_back(p[1]);
        diameters.push_back(p[2]);
    }

    std::vector<std::vector<int32_t>> structure;
    _read(_g_mitochondria, _d_structure, 2, structure);

    auto& mitoSection = _properties.get_mut<Property::MitoSection>();
    mitoSection.reserve(mitoSection.size() + structure.size());
    for (auto& s : structure)
        mitoSection.emplace_back(Property::MitoSection::Type{s[0], s[1]});
}

}  // namespace h5
}  // namespace readers
}  // namespace morphio
