/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vasculatureHDF5.h"

#include <highfive/H5DataType.hpp>  // for HighFive::SilenceHDF5
#include <highfive/H5Utility.hpp>  // for HighFive::SilenceHDF5

namespace HighFive {
template <>
inline AtomicType<morphio::VascularSectionType>::AtomicType() {
    _hid = H5Tcopy(H5T_NATIVE_INT);
}
}  // namespace HighFive

namespace morphio {
namespace readers {
namespace h5 {

vasculature::property::Properties VasculatureHDF5::load() {
    try {
        HighFive::SilenceHDF5 silence;
        _file.reset(new HighFive::File(_uri, HighFive::File::ReadOnly));
    } catch (const HighFive::FileException& exc) {
        throw morphio::RawDataError(_write ? "Could not create vasculature file "
                                           : "Could not open vasculature file " + _uri + ": " +
                                                 exc.what());
    }
    _readDatasets();
    _readSections();
    _readPoints();
    _readSectionTypes();
    _readConnectivity();

    return _properties;
}

void VasculatureHDF5::_readDatasets() {
    HighFive::SilenceHDF5 silence;
    _points.reset(new HighFive::DataSet(_file->getDataSet("/points")));
    auto dataspace = _points->getSpace();
    _pointsDims = dataspace.getDimensions();
    if (_pointsDims.size() != 2 || _pointsDims[1] != 4) {
        throw morphio::RawDataError("Opening vasculature file '" + _file->getName() +
                                    "': bad number of dimensions in points dataspace");
    }
    _sections.reset(new HighFive::DataSet(_file->getDataSet("/structure")));
    dataspace = _sections->getSpace();
    _sectionsDims = dataspace.getDimensions();
    if (_sectionsDims.size() != 2 || _sectionsDims[1] != 2) {
        throw morphio::RawDataError("Opening vasculature file '" + _file->getName() +
                                    "': bad number of dimensions in structure dataspace");
    }
    _connectivity.reset(new HighFive::DataSet(_file->getDataSet("/connectivity")));
    dataspace = _connectivity->getSpace();
    _conDims = dataspace.getDimensions();
    if (_conDims.size() != 2 || _conDims[1] != 2) {
        throw morphio::RawDataError("Opening vasculature file '" + _file->getName() +
                                    "': bad number of dimensions in connectivity dataspace");
    }
}

void VasculatureHDF5::_readPoints() {
    auto& points = _properties.get_mut<vasculature::property::Point>();
    auto& diameters = _properties.get_mut<vasculature::property::Diameter>();

    std::vector<std::vector<morphio::floatType>> vec;
    vec.resize(_pointsDims[0]);
    _points->read(vec);
    for (const auto& p : vec) {
        points.push_back({p[0], p[1], p[2]});
        diameters.push_back(p[3]);
    }
}

void VasculatureHDF5::_readSections() {
    auto& sections = _properties.get_mut<vasculature::property::VascSection>();
    auto selection = _sections->select({0, 0}, {_sectionsDims[0], 1});

    std::vector<unsigned int> vec;
    vec.resize(_sectionsDims[0]);
    selection.read(vec);

    for (auto p : vec) {
        sections.push_back(p);
    }
}

void VasculatureHDF5::_readSectionTypes() {
    auto& types = _properties.get_mut<vasculature::property::SectionType>();

    auto selection = _sections->select({0, 1}, {_sectionsDims[0], 1});
    types.resize(_sectionsDims[0]);
    selection.read(types);
    for (int type : types) {
        if (type > SECTION_CUSTOM || type < 0) {
            throw morphio::RawDataError(_err.ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(
                0, static_cast<VascularSectionType>(type)));
        }
    }
}

void VasculatureHDF5::_readConnectivity() {
    std::vector<std::vector<unsigned int>> vec;
    vec.resize(_conDims[0]);
    _connectivity->read(vec);
    auto& con = _properties._connectivity;
    for (auto& v : vec) {
        con.push_back({v[0], v[1]});
    }
}
}  // namespace h5
}  // namespace readers
}  // namespace morphio
