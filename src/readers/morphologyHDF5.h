#pragma once
#include <memory> // std::unique_ptr
#include <string> // std::string
#include <vector> // std::vector

#include <morphio/properties.h>
#include <morphio/types.h>
#include <morphio/errorMessages.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>

namespace morphio {
namespace readers {
namespace h5 {
Property::Properties load(const URI& uri);

class MorphologyHDF5
{
public:
    MorphologyHDF5(const std::string& uri) : _err(uri), _uri(uri){}
    virtual ~MorphologyHDF5();
    Property::Properties load();

private:
    void _checkVersion(const std::string& source);
    void _selectRepairStage();
    void _resolveV1();
    bool _readV11Metadata();
    bool _readV2Metadata();
    HighFive::DataSet _getStructureDataSet(size_t nSections);
    void _readPoints(int);
    int _readSections();
    void _readSectionTypes();
    void _readPerimeters(int);
    void _readMitochondria();

    template <typename T>
    void _read(const std::string& group,
               const std::string& _dataset,
               MorphologyVersion version,
               unsigned int expectedDimension,
               T& data);

    std::unique_ptr<HighFive::File> _file;

    std::unique_ptr<HighFive::DataSet> _points;
    std::vector<size_t> _pointsDims;

    std::unique_ptr<HighFive::DataSet> _sections;
    std::vector<size_t> _sectionsDims;

    std::string _stage;
    Property::Properties _properties;
    bool _write;
    ErrorMessages _err;
    std::string _uri;
};
} // namespace h5
} // namespace readers
} // namespace morphio
