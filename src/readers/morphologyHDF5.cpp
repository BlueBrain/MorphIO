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

#include <highfive/H5Utility.hpp> // HighFive::SilenceHDF5

namespace {
// v1 & v2
const std::string _d_points("points");
const size_t _pointColumns = 4;

// v1
const std::string _d_structure("/structure");
const size_t _structureV1Columns = 3;

// v1.1
const std::string _g_metadata("/metadata");
const std::string _a_family("cell_family");
const std::string _d_perimeters("/perimeters");
const std::string _g_mitochondria("/organelles/mitochondria");

// v1.1 & v2
const std::string _a_version("version");

// v2
const std::string _g_structure("structure");
const size_t _structureV2Columns = 2;
const std::string _g_root("neuron1");
const std::string _d_type("sectiontype");
const std::string _a_apical("apical");
} // namespace

namespace morphio {
namespace readers {
namespace h5 {
Property::Properties load(const std::string& uri)
{
    return MorphologyHDF5(uri).load();
}

Property::Properties MorphologyHDF5::load()
{
    _stage = "repaired";

    try {
        HighFive::SilenceHDF5 silence;
        _file.reset(new HighFive::File(_uri, HighFive::File::ReadOnly));
    } catch (const HighFive::FileException& exc) {
        LBTHROW(morphio::RawDataError(_write
                                      ? "Could not create morphology file "
                                      : "Could not open morphology file " + _uri + ": " + exc.what()));
    }
    _checkVersion(_uri);
    _selectRepairStage();
    int firstSectionOffset = _readSections();
    _readPoints(firstSectionOffset);
    _readSectionTypes();
    _readPerimeters(firstSectionOffset);
    _readMitochondria();

    return _properties;
}

void MorphologyHDF5::_checkVersion(const std::string& source)
{
    if (_readV11Metadata())
        return;

    if (_readV2Metadata())
        return;

    try {
        _resolveV1();
        _properties._cellLevel._version = MORPHOLOGY_VERSION_H5_1;
        return;
    } catch (...) {
        LBTHROW(
            morphio::RawDataError("Unknown morphology file format for file " +
                                  source));
    }
}

void MorphologyHDF5::_selectRepairStage()
{
    if (_properties.version() != MORPHOLOGY_VERSION_H5_2)
        return;

    for (const auto& stage : {"repaired", "unraveled", "raw"}) {
        try {
            HighFive::SilenceHDF5 silence;
            _file->getDataSet("/" + _g_root + "/" + stage + "/" + _d_points);
            _stage = stage;
            return;
        } catch (const HighFive::DataSetException&) {
        }
    }
    _stage = "repaired";
}

void MorphologyHDF5::_resolveV1()
{
    HighFive::SilenceHDF5 silence;
    _points.reset(new HighFive::DataSet(_file->getDataSet("/" + _d_points)));
    auto dataspace = _points->getSpace();
    _pointsDims = dataspace.getDimensions();

    if (_pointsDims.size() != 2 || _pointsDims[1] != _pointColumns) {
        LBTHROW(morphio::RawDataError("Opening morphology file '" + _file->getName() +
                                      "': bad number of dimensions in 'points' dataspace"));
    }

    _sections.reset(new HighFive::DataSet(_file->getDataSet(_d_structure)));
    dataspace = _sections->getSpace();
    _sectionsDims = dataspace.getDimensions();
    if (_sectionsDims.size() != 2 || _sectionsDims[1] != _structureV1Columns) {
        LBTHROW(morphio::RawDataError("Opening morphology file '" + _file->getName() +
                                      "': bad number of dimensions in 'structure' dataspace"));
    }
}

bool MorphologyHDF5::_readV11Metadata()
{
    try {
        HighFive::SilenceHDF5 silence;
        const auto metadata = _file->getGroup(_g_metadata);
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
        LBTHROW(morphio::RawDataError(
            std::string("Error reading morphology metadata: ") + e.what()));
    }

    _resolveV1();
    return true;
}

bool MorphologyHDF5::_readV2Metadata()
{
    try {
        HighFive::SilenceHDF5 silence;
        const auto root = _file->getGroup(_g_root);
        const auto attr = root.getAttribute(_a_version);
        attr.read(_properties.version());
        if (_properties.version() == MORPHOLOGY_VERSION_H5_2)
            return true;
    } catch (const HighFive::Exception&) {
    }

    try {
        HighFive::SilenceHDF5 silence;
        _file->getGroup(_g_root);
        _properties._cellLevel._version = MORPHOLOGY_VERSION_H5_2;
        return true;
    } catch (const HighFive::Exception&) {
        return false;
    }
}

HighFive::DataSet MorphologyHDF5::_getStructureDataSet(size_t nSections)
{
    try {
        HighFive::SilenceHDF5 silence;
        return _file->getDataSet(_d_structure);
    } catch (const HighFive::DataSetException&) {
        return _file->createDataSet<int>(_d_structure,
            HighFive::DataSpace({nSections, 3}));
    }
}


/**
   Returns true if the neuron has no neurites
**/
static inline bool noNeurites(int firstSectionOffset)
{
    return (firstSectionOffset == -1);
}


void MorphologyHDF5::_readPoints(int firstSectionOffset)
{
    auto& points = _properties.get<Property::Point>();
    auto& diameters = _properties.get<Property::Diameter>();

    auto& somaPoints = _properties._somaLevel._points;
    auto& somaDiameters = _properties._somaLevel._diameters;

    if (_properties.version() == MORPHOLOGY_VERSION_H5_2) {
        auto dataset = [this]() {
            std::string path = "/" + _g_root + "/" + _stage + "/" + _d_points;
            try {
                return _file->getDataSet(path);
            } catch (HighFive::DataSetException&) {
                LBTHROW(MorphioError(
                    "Could not open " + path + " dataset for morphology file " + _file->getName() +
                    " repair stage " + _stage));
            }
        }();

        const auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != 2 || dims[1] != _pointColumns) {
            LBTHROW(MorphioError(
                "Reading morphology file '" + _file->getName() +
                "': bad number of dimensions in 'points' dataspace"));
        }
        std::vector<std::vector<float>> vec(dims[0]);
        dataset.read(vec);

        std::size_t offset = vec.size();
        if (firstSectionOffset >= 0) {
            offset = static_cast<size_t>(firstSectionOffset);
        }

        somaPoints.reserve(somaPoints.size() + offset);
        somaDiameters.reserve(somaDiameters.size() + offset);
        for (std::size_t i = 0; i < offset; ++i) {
            const auto& p = vec[i];
            somaPoints.emplace_back(Point{p[0], p[1], p[2]});
            somaDiameters.emplace_back(p[3]);
        }

        if (!noNeurites(firstSectionOffset)) {
            size_t size = (points.size() + vec.size() -
                           static_cast<unsigned int>(firstSectionOffset));
            points.reserve(size);
            diameters.reserve(size);
            for (std::size_t i = offset; i < vec.size(); ++i) {
                const auto& p = vec[i];
                points.emplace_back(Point{p[0], p[1], p[2]});
                diameters.emplace_back(p[3]);
            }
        }
        return;
    }

    std::vector<std::vector<float>> vec;
    vec.resize(_pointsDims[0]);
    _points->read(vec);

    std::size_t offset = vec.size();
    if (firstSectionOffset >= 0) {
        offset = static_cast<unsigned int>(firstSectionOffset);
    }

    somaPoints.reserve(somaPoints.size() + offset);
    somaDiameters.reserve(somaDiameters.size() + offset);
    for (std::size_t i = 0; i < offset; ++i) {
        const auto& p = vec[i];
        somaPoints.emplace_back(Point{p[0], p[1], p[2]});
        somaDiameters.emplace_back(p[3]);
    }

    points.reserve(points.size() + vec.size() - offset);
    diameters.reserve(diameters.size() + vec.size() - offset);
    for (std::size_t i = offset; i < vec.size(); ++i) {
        const auto& p = vec[i];
        points.emplace_back(Point{p[0], p[1], p[2]});
        diameters.emplace_back(p[3]);
    }
}

int MorphologyHDF5::_readSections()
{
    auto& sections = _properties.get<Property::Section>();

    if (_properties.version() == MORPHOLOGY_VERSION_H5_2) {
        // fixes BBPSDK-295 by restoring old BBPSDK 0.13 implementation
        HighFive::SilenceHDF5 silence;
        auto dataset = [this]() {
            std::string path = "/" + _g_root + "/" + _g_structure + "/" + _stage;
            try {
                return _file->getDataSet(path);
            } catch (HighFive::DataSetException&) {
                if (_stage == "unraveled") {
                    std::string raw_path = "/" + _g_root + "/" + _g_structure + "/raw";
                    try {
                        return _file->getDataSet(raw_path);
                    } catch (HighFive::DataSetException&) {
                        LBTHROW(MorphioError("Could not find unraveled structure neither at " +
                                             path + " or " + raw_path +
                                             " for dataset for morphology file " +
                                             _file->getName() + " repair stage " + _stage));
                    }
                } else {
                    LBTHROW(MorphioError("Could not open " + path +
                                         " dataset for morphology file " + _file->getName() +
                                         " repair stage " + _stage));
                }
            }
        }();

        _sections.reset(new HighFive::DataSet(dataset));

        const auto dims = dataset.getSpace().getDimensions();

        if (dims.size() != 2 || dims[1] != _structureV2Columns) {
            LBTHROW(MorphioError(
                "Reading morphology file '" + _file->getName() +
                "': bad number of dimensions in 'structure' dataspace"));
        }

        std::vector<std::vector<int>> vec;
        vec.resize(dims[0] - 1);
        dataset.read(vec);
        int firstSectionOffset = vec[1][0];
        sections.reserve(sections.size() + vec.size() - 1);
        bool skipFirst = true;
        for (const auto& p : vec) {
            if (skipFirst) {
                skipFirst = false;
                continue;
            }
            sections.emplace_back(Property::Section::Type{p[0] - firstSectionOffset, p[1] - 1});
        }

        return firstSectionOffset;
    }

    auto selection = _sections->select({0, 0}, {_sectionsDims[0], 2}, {1, 2});

    std::vector<std::vector<int>> vec;
    vec.resize(_sectionsDims[0]);
    selection.read(vec);

    if (vec.size() < 2) // Neuron without any neurites
        return -1;

    int firstSectionOffset = vec[1][0];

    sections.reserve(sections.size() + vec.size() - 1);
    bool skipFirst = true; // Skipping soma section
    for (const auto& p : vec) {
        if (skipFirst) {
            skipFirst = false;
            continue;
        }
        sections.emplace_back(Property::Section::Type{p[0] - firstSectionOffset, p[1] - 1});
    }

    return firstSectionOffset;
}

void MorphologyHDF5::_readSectionTypes()
{
    auto& types = _properties.get<Property::SectionType>();

    if (_properties.version() == MORPHOLOGY_VERSION_H5_2) {
        auto dataset = [this]() {
            std::string path = "/" + _g_root + "/" + _g_structure + "/" + _d_type;
            try {
                return _file->getDataSet(path);
            } catch (HighFive::DataSetException&) {
                LBTHROW(MorphioError("Could not open " + path +
                                     " dataset for morphology file " + _file->getName()));
            }
        }();

        const auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != 2 || dims[1] != 1) {
            LBTHROW(MorphioError("Reading morphology file '" + _file->getName() +
                                 "': bad number of dimensions in 'sectiontype' dataspace"));
        }

        types.resize(dims[0]);
        dataset.read(types);
        types.erase(types.begin());// remove soma type
        for (int type : types) {
            if (type > SECTION_CUSTOM_START || type < 0) {
                LBTHROW(morphio::RawDataError(
                        _err.ERROR_UNSUPPORTED_SECTION_TYPE(0, static_cast<SectionType>(type))));
            }
        }
        return;
    }

    auto selection = _sections->select({0, 1}, {_sectionsDims[0], 1});
    types.resize(_sectionsDims[0]);
    selection.read(types);
    types.erase(types.begin()); // remove soma type
    for (int type : types) {
        if (type > SECTION_CUSTOM_START || type < 0) {
            LBTHROW(morphio::RawDataError(
                    _err.ERROR_UNSUPPORTED_SECTION_TYPE(0, static_cast<SectionType>(type))));
        }
    }
}


void MorphologyHDF5::_readPerimeters(int firstSectionOffset)
{
    if (_properties.version() != MORPHOLOGY_VERSION_H5_1_1 || noNeurites(firstSectionOffset))
        return;

    try {
        HighFive::SilenceHDF5 silence;
        HighFive::DataSet dataset = _file->getDataSet(_d_perimeters);

        auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != 1) {
            LBTHROW(MorphioError("Reading morphology file '" + _file->getName() +
                                 "': bad number of dimensions in 'perimeters' dataspace"));
        }

        std::vector<float> perimeters;
        perimeters.resize(dims[0]);
        dataset.read(perimeters);
        _properties.get<Property::Perimeter>().assign(perimeters.begin() + firstSectionOffset,
            perimeters.end());
    } catch (...) {
        if (_properties._cellLevel._cellFamily == FAMILY_GLIA)
            LBTHROW(
                MorphioError("No empty perimeters allowed for glia "
                                   "morphology"));
    }
}

template <typename T>
void MorphologyHDF5::_read(const std::string& groupName,
    const std::string& _dataset,
    MorphologyVersion version,
    unsigned int expectedDimension, T& data)
{
    if (_properties.version() != version)
        return;
    try {
        const auto group = _file->getGroup(groupName);

        HighFive::DataSet dataset = group.getDataSet(_dataset);

        auto dims = dataset.getSpace().getDimensions();
        if (dims.size() != expectedDimension) {
            LBTHROW(MorphioError("Reading morphology file '" + _file->getName() +
                                 "': bad number of dimensions in 'perimeters' dataspace"));
        }

        data.resize(dims[0]);
        dataset.read(data);
    } catch (...) {
        if (_properties._cellLevel._cellFamily == FAMILY_GLIA)
            LBTHROW(
                MorphioError("No empty perimeters allowed for glia "
                                   "morphology"));
    }
}

void MorphologyHDF5::_readMitochondria()
{
    {
        HighFive::SilenceHDF5 silence;

        try {
            const auto group = _file->getGroup(_g_mitochondria);
        } catch (HighFive::GroupException&) {
            return;
        }
    }

    std::vector<std::vector<float>> points;
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
    _read(_g_mitochondria, "structure", MORPHOLOGY_VERSION_H5_1_1, 2,
        structure);

    auto& mitoSection = _properties.get<Property::MitoSection>();
    mitoSection.reserve(mitoSection.size() + structure.size());
    for (auto& s : structure)
        mitoSection.emplace_back(Property::MitoSection::Type{s[0], s[1]});
}

} // namespace h5
} // namespace readers
} // namespace morphio
