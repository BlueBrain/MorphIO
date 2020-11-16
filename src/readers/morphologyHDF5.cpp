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
// v1 & v2
const std::string _d_points("points");
constexpr size_t _pointColumns = 4;

// v1
const std::string _d_structure("structure");
const size_t _structureV1Columns = 3;

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

// v1.1 & v2
const std::string _a_version("version");

// v2
const std::string _g_structure("structure");
constexpr size_t _structureV2Columns = 2;
const std::string _g_root("neuron1");
const std::string _d_type("sectiontype");
const std::string _a_apical("apical");
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
    _stage = "repaired";

    _checkVersion(_uri);
    _selectRepairStage();
    int firstSectionOffset = _readSections();
    _readPoints(firstSectionOffset);
    _readPerimeters(firstSectionOffset);
    _readMitochondria();
    _readEndoplasmicReticulum();

    return _properties;
}

MorphologyHDF5::MorphologyHDF5(const HighFive::Group& group)
    : _group(group)
    , _uri("HDF5 Group") {}

void MorphologyHDF5::_checkVersion(const std::string& source) {
    if (_readV11Metadata())
        return;

    if (_readV2Metadata())
        return;

    try {
        _resolveV1();
        _properties._cellLevel._version = MORPHOLOGY_VERSION_H5_1;
        return;
    } catch (...) {
        throw morphio::RawDataError("Unknown morphology format in " + source);
    }
}

void MorphologyHDF5::_selectRepairStage() {
    if (_properties.version() != MORPHOLOGY_VERSION_H5_2)
        return;

    for (const auto& stage : {"repaired", "unraveled", "raw"}) {
        try {
            HighFive::SilenceHDF5 silence;
            _group.getDataSet("/" + _g_root + "/" + stage + "/" + _d_points);
            _stage = stage;
            return;
        } catch (const HighFive::DataSetException&) {
        }
    }
    _stage = "repaired";
}

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

bool MorphologyHDF5::_readV11Metadata() {
    try {
        HighFive::SilenceHDF5 silence;
        const auto metadata = _group.getGroup(_g_metadata);
        const auto attr = metadata.getAttribute(_a_version);

        uint32_t version[2];
        attr.read(version);
        if (version[0] != 1 || version[1] != 1)
            return false;

        _properties._cellLevel._version = MORPHOLOGY_VERSION_H5_1_1;

        const auto familyAttr = metadata.getAttribute(_a_family);
        uint32_t family;
        familyAttr.read(family);
        _properties._cellLevel._cellFamily = static_cast<CellFamily>(family);
    } catch (const HighFive::GroupException&) {
        return false;
    } catch (const HighFive::Exception& e) {
        // All other exceptions are not expected because if the metadata
        // group exits it must contain at least the version, and for
        // version 1.1 it must contain the family.
        throw morphio::RawDataError("Error reading morphology " + _uri + "  metadata." + e.what());
    }

    _resolveV1();
    return true;
}

bool MorphologyHDF5::_readV2Metadata() {
    try {
        HighFive::SilenceHDF5 silence;
        const auto root = _group.getGroup(_g_root);
        const auto attr = root.getAttribute(_a_version);
        attr.read(_properties._cellLevel._version);
        if (_properties.version() == MORPHOLOGY_VERSION_H5_2)
            return true;
    } catch (const HighFive::Exception&) {
    }

    try {
        HighFive::SilenceHDF5 silence;
        _group.getGroup(_g_root);
        _properties._cellLevel._version = MORPHOLOGY_VERSION_H5_2;
        return true;
    } catch (const HighFive::Exception&) {
        return false;
    }
}

HighFive::DataSet MorphologyHDF5::_getStructureDataSet(size_t nSections) {
    try {
        HighFive::SilenceHDF5 silence;
        return _group.getDataSet(_d_structure);
    } catch (const HighFive::DataSetException&) {
        return _group.createDataSet<int>(_d_structure, HighFive::DataSpace({nSections, 3}));
    }
}


/**
   Returns true if the neuron has neurites (for MORPHOLOGY_VERSION_H5_2)
**/
static inline bool v2HasNeurites(int firstSectionOffset) {
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

    if (_properties.version() == MORPHOLOGY_VERSION_H5_2) {
        auto dataset = [this]() {
            std::string path = "/" + _g_root + "/" + _stage + "/" + _d_points;
            try {
                return _group.getDataSet(path);
            } catch (HighFive::DataSetException&) {
                throw(MorphioError("Could not open " + path + " dataset " + " repair stage " +
                                   _stage));
            }
        }();

        const auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != 2 || dims[1] != _pointColumns) {
            throw(MorphioError("'Error reading morphologies: " + _uri +
                               " bad number of dimensions in 'points' dataspace"));
        }
        std::vector<std::array<morphio::floatType, _pointColumns>> vec(dims[0]);
        if (vec.size() > 0) {
            dataset.read(vec.front().data());
        }
        loadPoints(vec, v2HasNeurites(firstSectionOffset));
    } else {
        std::vector<std::array<morphio::floatType, _pointColumns>> vec(_pointsDims[0]);
        if (vec.size() > 0) {
            _points->read(vec.front().data());
        }
        loadPoints(vec, std::size_t(firstSectionOffset) < _pointsDims[0]);
    }
}

inline int MorphologyHDF5::_readSections() {
    return (_properties.version() == MORPHOLOGY_VERSION_H5_2) ? _readV2Sections()
                                                              : _readV1Sections();
}

int MorphologyHDF5::_readV1Sections() {
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

int MorphologyHDF5::_readV2Sections() {
    auto& sections = _properties.get<Property::Section>();
    auto& types = _properties.get<Property::SectionType>();

    // fixes BBPSDK-295 by restoring old BBPSDK 0.13 implementation
    HighFive::SilenceHDF5 silence;
    auto dataset = [this]() {
        std::string path = "/" + _g_root + "/" + _g_structure + "/" + _stage;
        try {
            return _group.getDataSet(path);
        } catch (HighFive::DataSetException&) {
            if (_stage == "unraveled") {
                std::string raw_path = "/" + _g_root + "/" + _g_structure + "/raw";
                try {
                    return _group.getDataSet(raw_path);
                } catch (HighFive::DataSetException&) {
                    throw(MorphioError("Could not find unraveled structure neither at " + path +
                                       " or " + raw_path + " for dataset for morphology '" + _uri +
                                       "' repair stage " + _stage));
                }
            } else {
                throw(MorphioError("Could not open " + path + " dataset for morphology '" + _uri +
                                   "' repair stage " + _stage));
            }
        }
    }();

    auto dataset_types = [this]() {
        std::string path = "/" + _g_root + "/" + _g_structure + "/" + _d_type;
        try {
            return _group.getDataSet(path);
        } catch (HighFive::DataSetException&) {
            throw(MorphioError("Could not open " + path + " dataset for morphology: " + _uri));
        }
    }();

    _sections.reset(new HighFive::DataSet(dataset));

    const auto dims = dataset.getSpace().getDimensions();

    if (dims.size() != 2 || dims[1] != _structureV2Columns) {
        throw(MorphioError("Error reading morphologies " + _uri +
                           " bad number of dimensions in 'structure' dataspace"));
    }

    std::vector<std::array<int, _structureV2Columns>> vec(dims[0]);
    if (vec.size() > 0) {
        dataset.read(vec.front().data());
    }
    dataset_types.read(types);

    int firstSectionOffset = vec[1][0];
    sections.reserve(sections.size() + vec.size() - 1);

    // The first contains soma related value so it is skipped
    for (size_t i = 1; i < vec.size(); ++i) {
        const auto& p = vec[i];
        const int& type = types[i];  // Explicit int for Clang (<0 comparison)

        if (type > SECTION_CUSTOM_START || type < 0) {
            throw morphio::RawDataError(
                _err.ERROR_UNSUPPORTED_SECTION_TYPE(0, static_cast<SectionType>(type)));
        }

        sections.emplace_back(Property::Section::Type{p[0] - firstSectionOffset, p[1] - 1});
    }

    types.erase(types.begin());  // remove soma type

    return firstSectionOffset;
}

void MorphologyHDF5::_readPerimeters(int firstSectionOffset) {
    if (_properties.version() != MORPHOLOGY_VERSION_H5_1_1 || !v2HasNeurites(firstSectionOffset))
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
                           MorphologyVersion version,
                           unsigned int expectedDimension,
                           T& data) {
    if (_properties.version() != version)
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
          MORPHOLOGY_VERSION_H5_1_1,
          1,
          _properties._endoplasmicReticulumLevel._sectionIndices);
    _read(_g_endoplasmic_reticulum,
          _d_volume,
          MORPHOLOGY_VERSION_H5_1_1,
          1,
          _properties._endoplasmicReticulumLevel._volumes);
    _read(_g_endoplasmic_reticulum,
          _d_surface_area,
          MORPHOLOGY_VERSION_H5_1_1,
          1,
          _properties._endoplasmicReticulumLevel._surfaceAreas);
    _read(_g_endoplasmic_reticulum,
          _d_filament_count,
          MORPHOLOGY_VERSION_H5_1_1,
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
    _read(_g_mitochondria, _d_points, MORPHOLOGY_VERSION_H5_1_1, 2, points);

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
    _read(_g_mitochondria, "structure", MORPHOLOGY_VERSION_H5_1_1, 2, structure);

    auto& mitoSection = _properties.get<Property::MitoSection>();
    mitoSection.reserve(mitoSection.size() + structure.size());
    for (auto& s : structure)
        mitoSection.emplace_back(Property::MitoSection::Type{s[0], s[1]});
}

}  // namespace h5
}  // namespace readers
}  // namespace morphio
