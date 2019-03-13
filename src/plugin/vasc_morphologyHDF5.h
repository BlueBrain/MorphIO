#pragma once

#include <morphio/vascProperties.h>
#include <morphio/types.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Utility.hpp>

namespace morphio {
namespace plugin {
namespace h5 {
VasculatureProperty::Properties load_vasc(const URI& uri);

class VasculatureMorphologyHDF5
{
public:
    virtual ~VasculatureMorphologyHDF5();
    VasculatureProperty::Properties load(const URI& uri);

private:
    void _resolve();
    //HighFive::DataSet _getStructureDataSet(size_t nSections);
    void _readPoints();
    void _readSections();
    void _readSectionTypes();
    void _readConnectivity();

    std::unique_ptr<HighFive::File> _file;

    std::unique_ptr<HighFive::DataSet> _points;
    std::vector<size_t> _pointsDims;

    std::unique_ptr<HighFive::DataSet> _sections;
    std::vector<size_t> _sectionsDims;

    std::unique_ptr<HighFive::DataSet> _connectivity;
    std::vector<size_t> _conDims;

    VasculatureProperty::Properties _properties;
    bool _write;
};
}
}
}
