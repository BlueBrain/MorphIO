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

#include "utilsHDF5.h"

#include <highfive/H5Utility.hpp>  // HighFive::SilenceHDF5

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

// } v1

//{ v2
const std::string _g_v2root("neuron1");
//} v2

}  // namespace

namespace morphio {
namespace readers {
namespace h5 {

MorphologyHDF5::MorphologyHDF5(const HighFive::Group& group)
    : _group(group)
    , _uri("HDF5 Group") {}

Property::Properties load(const std::string& uri) {
    try {
        HighFive::SilenceHDF5 silence;
        auto file = HighFive::File(uri, HighFive::File::ReadOnly);
        return MorphologyHDF5(file.getGroup("/")).load();

    } catch (const HighFive::FileException& exc) {
        throw RawDataError("Could not open morphology file " + uri + ": " + exc.what());
    }
}

Property::Properties load(const HighFive::Group& group) {
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

            if (majorVersion == 1 && (minorVersion == 1 || minorVersion == 2)) {
                uint32_t family;
                metadata.getAttribute(_a_family).read(family);
                _properties._cellLevel._cellFamily = static_cast<CellFamily>(family);
            } else {
                throw RawDataError("Error in " + source +
                                   "\nUnsupported h5 version: " + std::to_string(majorVersion) +
                                   "." + std::to_string(minorVersion) +
                                   "See "
                                   "https://bbpteam.epfl.ch/documentation/projects/"
                                   "Morphology%20Documentation/latest/"
                                   "index.html for the list of supported versions.");
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

    const auto pointsDataSet = _group.getDataSet(_d_points);
    const auto pointsDims = pointsDataSet.getSpace().getDimensions();
    const size_t numberPoints = pointsDims[0];

    if (pointsDims.size() != 2) {
        throw RawDataError("Opening morphology '" + _uri +
                           "': incorrect number of dimensions in 'points'.");
    } else if (pointsDims[1] != pointColumns) {
        throw RawDataError("Opening morphology '" + _uri +
                           "': incorrect number of columns for points");
    }

    std::vector<std::array<floatType, pointColumns>> hd5fData(numberPoints);

    if (!hd5fData.empty()) {
        pointsDataSet.read(hd5fData.front().data());
    }

    const bool hasSoma = firstSectionOffset != 0;
    const bool hasNeurites = static_cast<size_t>(firstSectionOffset) < numberPoints;
    const size_t somaPointCount = hasNeurites ? static_cast<size_t>(firstSectionOffset)
                                              : hd5fData.size();

    auto& somaPoints = _properties._somaLevel._points;
    auto& somaDiameters = _properties._somaLevel._diameters;

    if (hasSoma) {
        somaPoints.resize(somaPointCount);
        somaDiameters.resize(somaPointCount);

        for (size_t i = 0; i < somaPointCount; ++i) {
            const auto& p = hd5fData[i];
            somaPoints[i] = {p[0], p[1], p[2]};
            somaDiameters[i] = p[3];
        }
    }

    auto& points = _properties.get_mut<Property::Point>();
    auto& diameters = _properties.get_mut<Property::Diameter>();

    if (hasNeurites) {
        const size_t size = (hd5fData.size() - somaPointCount);
        points.resize(size);
        diameters.resize(size);
        for (size_t i = somaPointCount; i < hd5fData.size(); ++i) {
            const auto& p = hd5fData[i];
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

    const auto structure = _group.getDataSet(_d_structure);
    const auto dims = structure.getSpace().getDimensions();

    if (dims.size() != 2 || dims[1] != structureV1Columns) {
        throw(RawDataError("Error reading morphologies " + _uri +
                           " bad number of dimensions in 'structure' dataspace"));
    }

    std::vector<std::array<int, structureV1Columns>> vec(dims[0]);
    if (dims[0] > 0) {
        structure.read(vec.front().data());
    }

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
            throw(RawDataError("Error reading morphology " + _uri +
                               ": it has soma section that doesn't come first"));
        } else if (hasSoma && type == SECTION_SOMA) {
            throw(RawDataError("Error reading morphology " + _uri +
                               ": it has multiple soma sections"));
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
    _read("/", _d_perimeters, 1, perimeters);
    perimeters.erase(perimeters.begin(), perimeters.begin() + firstSectionOffset);
}


template <typename T>
void MorphologyHDF5::_read(const std::string& groupName,
                           const std::string& datasetName,
                           unsigned int expectedDimension,
                           T& data) {
    assert(_group.exist(groupName));
    const auto group = _group.getGroup(groupName);

    assert(group.exist(datasetName));
    const HighFive::DataSet dataset = group.getDataSet(datasetName);

    const auto dims = dataset.getSpace().getDimensions();
    if (dims.size() != expectedDimension) {
        throw(RawDataError("Reading morphology '" + _uri + "': bad number of dimensions in " +
                           datasetName));
    }

    data.resize(dims[0]);
    dataset.read(data);
}

void MorphologyHDF5::_readEndoplasmicReticulum() {
    if (!_group.exist(_g_endoplasmic_reticulum)) {
        return;
    }

    const auto group = _group.getGroup(_g_endoplasmic_reticulum);

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

    const auto group = _group.getGroup(_g_mitochondria);

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
