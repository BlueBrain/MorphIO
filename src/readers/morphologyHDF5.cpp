/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#include "morphologyHDF5.h"

#include "utilsHDF5.h"

#include <highfive/H5Utility.hpp>  // HighFive::SilenceHDF5

namespace {
const std::string _d_points("points");
constexpr size_t _pointColumns = 4;

// v1
const std::string _d_structure("structure");
const size_t _structureV1Columns = 3;
const std::string _a_version("version");


// v1.1
const std::string _g_metadata("metadata");
const std::string _a_family("cell_family");
const std::string _d_perimeters("perimeters");
const std::string _g_mitochondria("organelles/mitochondria");

// endoplasmic reticulum
const std::string _g_endoplasmic_reticulum("/organelles/endoplasmic_reticulum");
const std::string _d_section_index("section_index");
const std::string _d_volume("volume");
const std::string _d_surface_area("surface_area");
const std::string _d_filament_count("filament_count");
const std::string _g_root("neuron1");
}  // namespace

namespace morphio {
namespace readers {
namespace h5 {

Property::Properties load(const std::string& uri) {
    try {
        HighFive::SilenceHDF5 silence;
        auto file = HighFive::File(uri, HighFive::File::ReadOnly);
        return MorphologyHDF5(file.getGroup("/")).load();

    } catch (const HighFive::FileException& exc) {
        throw morphio::RawDataError("Could not open morphology file " + uri + ": " + exc.what());
    }
}

Property::Properties load(const HighFive::Group& group) {
    return MorphologyHDF5(group).load();
}

Property::Properties MorphologyHDF5::load() {
    _readMetadata(_uri);
    int firstSectionOffset = _readSections();
    _readPoints(firstSectionOffset);
    _readPerimeters(firstSectionOffset);
    _readMitochondria();
    _readEndoplasmicReticulum();
    std::get<0>(_properties._cellLevel._version) = "h5";

    return _properties;
}

MorphologyHDF5::MorphologyHDF5(const HighFive::Group& group)
    : _group(group)
    , _uri("HDF5 Group") {}


void MorphologyHDF5::_resolveV1() {
    HighFive::SilenceHDF5 silence;
    _points.reset(new HighFive::DataSet(_group.getDataSet(_d_points)));
    auto dataspace = _points->getSpace();
    _pointsDims = dataspace.getDimensions();

    if (_pointsDims.size() != 2 || _pointsDims[1] != _pointColumns) {
        throw morphio::RawDataError("Opening morphology '" + _uri +
                                    "': bad number of dimensions in 'points' dataspace.");
    }

    _sections.reset(new HighFive::DataSet(_group.getDataSet(_d_structure)));
}

/* Heuristic to determine the version
 - Look for the version field (which exists only for h5v1.1 and h5v1.2)
 - Try to resolve the h5v1 point dataset, if it works and no version field was previously found
   then it must be h5v1.0
   Returns in case of successful resolution
 - If the resolution fails, tries to find the h5v2 version field (which is located elsewhere)
   throw a custom message if h5v2 is detected
 - in any other case, throws a generic message
*/
void MorphologyHDF5::_readMetadata(const std::string& source) {
    // h5v1.1 and h5v2 attempt
    try {
        HighFive::SilenceHDF5 silence;
        const auto metadata = _group.getGroup(_g_metadata);
        const auto attr = metadata.getAttribute(_a_version);

        uint32_t versions[2];
        attr.read(&versions);
        std::get<1>(_properties._cellLevel._version) = versions[0];
        std::get<2>(_properties._cellLevel._version) = versions[1];
        const auto majorVersion = _properties._cellLevel.majorVersion();
        const auto minorVersion = _properties._cellLevel.minorVersion();
        if (majorVersion == 1 && (minorVersion == 1 || minorVersion == 2)) {
            uint32_t family;
            const auto familyAttr = metadata.getAttribute(_a_family);
            familyAttr.read(family);
            _properties._cellLevel._cellFamily = static_cast<CellFamily>(family);
        } else {
            throw morphio::RawDataError(
                "Error in " + source + "\nUnsupported h5 version: " + std::to_string(majorVersion) +
                "." + std::to_string(minorVersion) +
                "See "
                "https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/"
                "index.html for the list of supported versions.");
        }
    } catch (const HighFive::Exception&) {
    }

    try {
        _resolveV1();

        // The fact that the resolution passes and majorVersion is still uninitialized tells us
        // it is h5v1.0
        if (_properties._cellLevel.majorVersion() == 0) {
            std::get<1>(_properties._cellLevel._version) = 1;  // major version
            std::get<2>(_properties._cellLevel._version) = 0;  // minor version
            // Version 1.0 only support NEURON has a CellFamily.
            // Other CellFamily have been added in version 1.1:
            // https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/h5v1.html
            _properties._cellLevel._cellFamily = CellFamily::NEURON;
        }
        return;
    } catch (const HighFive::Exception&) {
    }


    // h5v2 attempt
    {
        uint32_t potential_h5v2_version;
        try {
            _group.getGroup(_g_root).getAttribute(_a_version).read(potential_h5v2_version);
        } catch (const HighFive::Exception&) {
        }

        // More informative throw for h5v2 format
        if (potential_h5v2_version == 2)
            throw morphio::RawDataError(
                "Error in " + source +
                "\nh5v2 is no longer supported, see: https://github.com/BlueBrain/MorphIO#H5v2");
    }

    throw morphio::RawDataError("Error reading morphology " + _uri + "  metadata.");
}

HighFive::DataSet MorphologyHDF5::_getStructureDataSet(size_t nSections) {
    try {
        HighFive::SilenceHDF5 silence;
        return _group.getDataSet(_d_structure);
    } catch (const HighFive::DataSetException&) {
        return _group.createDataSet<int>(_d_structure, HighFive::DataSpace({nSections, 3}));
    }
}


static inline bool HasNeurites(int firstSectionOffset) {
    return (firstSectionOffset > -1);
}

void MorphologyHDF5::_readPoints(int firstSectionOffset) {
    auto& points = _properties.get<Property::Point>();
    auto& diameters = _properties.get<Property::Diameter>();

    auto& somaPoints = _properties._somaLevel._points;
    auto& somaDiameters = _properties._somaLevel._diameters;

    auto loadPoints =
        [&](const std::vector<std::array<morphio::floatType, _pointColumns>>& hd5fData,
            bool hasNeurites) {
            const std::size_t section_offset = hasNeurites ? std::size_t(firstSectionOffset)
                                                           : hd5fData.size();

            // points and diameters are PODs. Fastest to resize then assign values
            somaPoints.resize(somaPoints.size() + section_offset);
            somaDiameters.resize(somaDiameters.size() + section_offset);
            for (std::size_t i = 0; i < section_offset; ++i) {
                const auto& p = hd5fData[i];
                somaPoints[i] = {p[0], p[1], p[2]};
                somaDiameters[i] = p[3];
            }


            if (hasNeurites) {
                const size_t size = (points.size() + hd5fData.size() - section_offset);
                points.resize(size);
                diameters.resize(size);
                for (std::size_t i = section_offset; i < hd5fData.size(); ++i) {
                    const auto& p = hd5fData[i];
                    const std::size_t section_i = i - section_offset;
                    points[section_i] = {p[0], p[1], p[2]};
                    diameters[section_i] = p[3];
                }
            }
        };

    std::vector<std::array<morphio::floatType, _pointColumns>> vec(_pointsDims[0]);
    if (vec.size() > 0) {
        _points->read(vec.front().data());
    }
    loadPoints(vec, std::size_t(firstSectionOffset) < _pointsDims[0]);
}

int MorphologyHDF5::_readSections() {
    auto& sections = _properties.get<Property::Section>();
    auto& types = _properties.get<Property::SectionType>();

    // Important: The code used to split the reading of the sections and types
    //            into two separate fine-grained H5 selections. This does not
    //            reduce the number of I/O operations, but increases them by
    //            forcing HDF5 + MPI-IO to read in 4-byte groups. Thus, we now
    //            read the whole dataset at once, and split it in memory.

    const auto dims = _sections->getSpace().getDimensions();

    if (dims.size() != 2 || dims[1] != _structureV1Columns) {
        throw(MorphioError("Error reading morphologies " + _uri +
                           " bad number of dimensions in 'structure' dataspace"));
    }

    std::vector<std::array<int, _structureV1Columns>> vec(dims[0]);
    if (vec.size() > 0) {
        _sections->read(vec.front().data());
    }

    if (vec.size() < 2)  // Neuron without any neurites
        return -1;

    int firstSectionOffset = vec[1][0];
    sections.reserve(sections.size() + vec.size() - 1);
    types.reserve(vec.size() - 1);  // remove soma type

    // The first contains soma related value so it is skipped
    for (size_t i = 1; i < vec.size(); ++i) {
        const auto& p = vec[i];
        const int& type = p[1];  // Explicit int for Clang (<0 comparison)

        if (type > SECTION_CUSTOM_START || type < 0) {
            throw morphio::RawDataError(
                _err.ERROR_UNSUPPORTED_SECTION_TYPE(0, static_cast<SectionType>(type)));
        }

        sections.emplace_back(Property::Section::Type{p[0] - firstSectionOffset, p[2] - 1});
        types.emplace_back(static_cast<SectionType>(type));
    }

    return firstSectionOffset;
}

void MorphologyHDF5::_readPerimeters(int firstSectionOffset) {
    // Perimeter information is available starting at v1.1
    if (!(_properties._cellLevel.majorVersion() == 1 && _properties._cellLevel.minorVersion() > 0 &&
          HasNeurites(firstSectionOffset)))
        return;

    try {
        HighFive::SilenceHDF5 silence;
        HighFive::DataSet dataset = _group.getDataSet(_d_perimeters);

        auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != 1) {
            throw(MorphioError("Error reading morhologies: " + _uri +
                               " bad number of dimensions in 'perimeters' dataspace"));
        }

        std::vector<morphio::floatType> perimeters;
        perimeters.resize(dims[0]);
        dataset.read(perimeters);
        _properties.get<Property::Perimeter>().assign(perimeters.begin() + firstSectionOffset,
                                                      perimeters.end());
    } catch (...) {
        if (_properties._cellLevel._cellFamily == GLIA)
            throw MorphioError("No empty perimeters allowed for glia morphology");
    }
}

template <typename T>
void MorphologyHDF5::_read(const std::string& groupName,
                           const std::string& _dataset,
                           unsigned int expectedDimension,
                           T& data) {
    if (_properties._cellLevel.majorVersion() != 1 || _properties._cellLevel.minorVersion() < 1)
        return;
    try {
        const auto group = _group.getGroup(groupName);

        HighFive::DataSet dataset = group.getDataSet(_dataset);

        auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != expectedDimension) {
            throw(MorphioError("Reading morhology '" + _uri +
                               "': bad number of dimensions in 'perimeters' dataspace"));
        }

        data.resize(dims[0]);
        dataset.read(data);
    } catch (...) {
        if (_properties._cellLevel._cellFamily == GLIA)
            throw MorphioError("No empty perimeters allowed for glia morphology");
    }
}

void MorphologyHDF5::_readEndoplasmicReticulum() {
    {
        HighFive::SilenceHDF5 silence;

        try {
            const auto group = _group.getGroup(_g_endoplasmic_reticulum);
        } catch (const HighFive::GroupException&) {
            return;
        }
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
    {
        HighFive::SilenceHDF5 silence;

        try {
            const auto group = _group.getGroup(_g_mitochondria);
        } catch (HighFive::GroupException&) {
            return;
        }
    }

    std::vector<std::vector<morphio::floatType>> points;
    _read(_g_mitochondria, _d_points, 2, points);

    auto& mitoSectionId = _properties.get<Property::MitoNeuriteSectionId>();
    auto& pathlength = _properties.get<Property::MitoPathLength>();
    auto& diameters = _properties.get<Property::MitoDiameter>();
    mitoSectionId.reserve(mitoSectionId.size() + points.size());
    pathlength.reserve(pathlength.size() + points.size());
    diameters.reserve(diameters.size() + points.size());
    for (const auto& p : points) {
        mitoSectionId.push_back(static_cast<uint32_t>(p[0]));
        pathlength.push_back(p[1]);
        diameters.push_back(p[2]);
    }

    std::vector<std::vector<int32_t>> structure;
    _read(_g_mitochondria, "structure", 2, structure);

    auto& mitoSection = _properties.get<Property::MitoSection>();
    mitoSection.reserve(mitoSection.size() + structure.size());
    for (auto& s : structure)
        mitoSection.emplace_back(Property::MitoSection::Type{s[0], s[1]});
}

}  // namespace h5
}  // namespace readers
}  // namespace morphio
