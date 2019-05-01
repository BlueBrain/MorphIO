#include "vasculatureHDF5.h"

#include "../detail/utilsHDF5.h"

#include <iostream>

namespace morphio {
namespace plugin {
namespace h5 {
using namespace vasculature;

property::Properties VasculatureHDF5::load(const URI& uri)
{
    try {
        HighFive::SilenceHDF5 silence;
        _file.reset(new HighFive::File(uri, HighFive::File::ReadOnly));
    } catch (const HighFive::FileException& exc) {
        LBTHROW(morphio::RawDataError(_write
                                          ? "Could not create vasculature file "
                                          : "Could not open vasculature file " + uri + ": " + exc.what()));
    }
    _readDatasets();
    _readSections();
    _readPoints();
    _readSectionTypes();
    _readConnectivity();

    return _properties;
}

VasculatureHDF5::~VasculatureHDF5()
{
    _points.reset();
    _sections.reset();
    _connectivity.reset();
    _file.reset();
}

void VasculatureHDF5::_readDatasets()
{
    HighFive::SilenceHDF5 silence;
    _points.reset(new HighFive::DataSet(_file->getDataSet("/points")));
    auto dataspace = _points->getSpace();
    _pointsDims = dataspace.getDimensions();
    if (_pointsDims.size() != 2 || _pointsDims[1] != 4) {
        LBTHROW(morphio::RawDataError("Opening vasculature file '" + _file->getName() + "': bad number of dimensions in "
                                                                                        "points dataspace"));
    }
    _sections.reset(new HighFive::DataSet(_file->getDataSet("/structure")));
    dataspace = _sections->getSpace();
    _sectionsDims = dataspace.getDimensions();
    if (_sectionsDims.size() != 2 || _sectionsDims[1] != 2) {
        LBTHROW(morphio::RawDataError("Opening vasculature file '" + _file->getName() + "': bad number of dimensions in "
                                                                                        "structure dataspace"));
    }
    _connectivity.reset(new HighFive::DataSet(_file->getDataSet("/connectivity")));
    dataspace = _connectivity->getSpace();
    _conDims = dataspace.getDimensions();
    if (_conDims.size() != 2 || _conDims[1] != 2) {
        LBTHROW(morphio::RawDataError("Opening vasculature file '" + _file->getName() + "': bad number of dimensions in "
                                                                                        "connectivity dataspace"));
    }
}

void VasculatureHDF5::_readPoints()
{
    auto& points = _properties.get<property::Point>();
    auto& diameters = _properties.get<property::Diameter>();

    std::vector<std::vector<float>> vec;
    vec.resize(_pointsDims[0]);
    _points->read(vec);
    for (unsigned int i = 0; i < vec.size(); ++i) {
        const auto& p = vec[i];
        points.push_back({p[0], p[1], p[2]});
        diameters.push_back(p[3]);
    }
}

void VasculatureHDF5::_readSections()
{
    auto& sections = _properties.get<property::VascSection>();

    auto selection = _sections->select({0, 0}, {_sectionsDims[0], 1});

    std::vector<unsigned int> vec;
    vec.resize(_sectionsDims[0]);
    selection.read(vec);

    for (auto p : vec) {
        sections.push_back(p);
    }
}

void VasculatureHDF5::_readSectionTypes()
{
    std::vector<VascularSectionType>& types = _properties.get<property::SectionType>();

    auto selection = _sections->select({0, 1}, {_sectionsDims[0], 1});
    types.resize(_sectionsDims[0]);
    selection.read(types);
}

void VasculatureHDF5::_readConnectivity()
{
    std::vector<std::vector<unsigned int>> vec;
    vec.resize(_conDims[0]);
    _connectivity->read(vec);
    auto& con = _properties._connectivity;
    for (size_t i = 0; i < vec.size(); ++i) {
        con.push_back({vec[i][0], vec[i][1]});
    }
}
}
}
}
