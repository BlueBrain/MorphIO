#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Utility.hpp>

namespace morphio {
namespace plugin {
namespace h5 {
Property::Properties load(const URI& uri);

class MorphologyHDF5
{
public:
    virtual ~MorphologyHDF5();
    Property::Properties load(const URI& uri);

private:
    void _checkVersion(const std::string& source);
    void _selectRepairStage();
    void _resolveV1();
    bool _readV11Metadata();
    bool _readV2Metadata();
    HighFive::DataSet _getStructureDataSet(size_t nSections);
    void _readPoints(unsigned int);
    int _readSections();
    void _readSectionTypes();
    void _readPerimeters(int);
    void _readMitochondria();
    template <typename T>
    void _read(const std::string& group, const std::string& _dataset,
        MorphologyVersion version, unsigned int expectedDimension,
        T& data);

    std::unique_ptr<HighFive::File> _file;

    std::unique_ptr<HighFive::DataSet> _points;
    std::vector<size_t> _pointsDims;

    std::unique_ptr<HighFive::DataSet> _sections;
    std::vector<size_t> _sectionsDims;

    std::string _stage;
    Property::Properties _properties;
    bool _write;
};
} // namespace h5
} // namespace plugin
} // namespace morphio
