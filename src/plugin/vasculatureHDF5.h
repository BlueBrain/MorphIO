#pragma once

#include <morphio/types.h>
#include <morphio/vasc/properties.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Utility.hpp>

namespace morphio {
namespace plugin {
namespace h5 {
using namespace morphio::vasculature;

class VasculatureHDF5
{
public:
    virtual ~VasculatureHDF5();
    property::Properties load(const URI& uri);

private:
    void _readDatasets();
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

    property::Properties _properties;
    bool _write;
};
}
}
}
