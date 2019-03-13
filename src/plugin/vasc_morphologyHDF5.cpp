#include "vasc_morphologyHDF5.h"

#include <iostream>

namespace morphio {
namespace plugin {
namespace h5 {
VasculatureProperty::Properties load_vasc(const URI& uri)
{
    return VasculatureMorphologyHDF5().load(uri);
}

VasculatureProperty::Properties VasculatureMorphologyHDF5::load(const URI& uri)
{
    try {
        HighFive::SilenceHDF5 silence;
        _file.reset(new HighFive::File(uri, HighFive::File::ReadOnly));
    } catch (const HighFive::FileException& exc) {
        LBTHROW(morphio::RawDataError(_write
                                      ? "Could not create morphology file "
                                      : "Could not open morphology file " + uri + ": " + exc.what()));
    }
    _resolve();
    _readSections(); std::cout << "Read sections \n";
    _readPoints(); std::cout <<"Read points \n";
    _readSectionTypes(); std::cout << "Read types \n";
    _readConnectivity(); std::cout <<"read connectivity \n";

    return _properties;
}

VasculatureMorphologyHDF5::~VasculatureMorphologyHDF5()
{
    _points.reset();
    _sections.reset();
    _connectivity.reset();
    _file.reset();
}

void VasculatureMorphologyHDF5::_resolve()
{
    HighFive::SilenceHDF5 silence;
    _points.reset(new HighFive::DataSet(_file->getDataSet("/points")));
    auto dataspace = _points->getSpace();
    _pointsDims = dataspace.getDimensions();
    if (_pointsDims.size() != 2 || _pointsDims[1] != 4) {
        LBTHROW(morphio::RawDataError("Opening morphology file '" + _file->getName() + "': bad number of dimensions in "
                                                                                       "points dataspace"));
    }
    _sections.reset(new HighFive::DataSet(_file->getDataSet("/structure")));
    dataspace = _sections->getSpace();
    _sectionsDims = dataspace.getDimensions();
    if (_sectionsDims.size() != 2 || _sectionsDims[1] != 2) {
        LBTHROW(morphio::RawDataError("Opening morphology file '" + _file->getName() + "': bad number of dimensions in "
                                                                                       "structure dataspace"));
    }
    _connectivity.reset(new HighFive::DataSet(_file->getDataSet("/connectivity")));
    dataspace = _connectivity->getSpace();
    _conDims = dataspace.getDimensions();
    if (_conDims.size() !=2 || _conDims[1] != 2) {
        LBTHROW(morphio::RawDataError("Opening morphology file '" + _file->getName() + "': bad number of dimensions in "
                                                                                       "connectivity dataspace"));
    }
}

void VasculatureMorphologyHDF5::_readPoints()
{
    std::cout << "reading points\n";
    auto& points = _properties.get<VasculatureProperty::Point>();
    auto& diameters = _properties.get<VasculatureProperty::Diameter>();

    std::vector<std::vector<float>> vec;
    vec.resize(_pointsDims[0]);
    std::cout << _pointsDims[0] << std::endl;
    _points->read(vec);
    for (unsigned int i = 0; i < vec.size(); ++i) {
        const auto& p = vec[i];
        points.push_back({p[0], p[1], p[2]});
        diameters.push_back(p[3]);
    }
}

void VasculatureMorphologyHDF5::_readSections()
{
    auto& sections = _properties.get<VasculatureProperty::VascSection>();

    auto selection = _sections->select({0, 0}, {_sectionsDims[0], 1});

    std::vector<int> vec;
    vec.resize(_sectionsDims[0]);
    std::cout << _sectionsDims[0] << std::endl;
    selection.read(vec);

    for (auto p : vec) {
        sections.push_back(p);
    }
}

void VasculatureMorphologyHDF5::_readSectionTypes()
{
    auto& types = _properties.get<VasculatureProperty::SectionType>();

    auto selection = _sections->select({0, 1}, {_sectionsDims[0], 1});
    types.resize(_sectionsDims[0]);
    selection.read(types);
    types.erase(types.begin());
}

void VasculatureMorphologyHDF5::_readConnectivity()
{
    std::vector<std::vector<int>> vec;
    vec.resize(_conDims[0]);
    _connectivity->read(vec);
    auto& con = _properties._connectivity;
    for (size_t i=0; i < vec.size(); ++i) {
        const auto& c = vec[i];
        con.push_back({c[0], c[1]});
    }
}


}
}
}