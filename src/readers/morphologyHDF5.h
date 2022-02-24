#pragma once
#include <string>  // std::string

#include <morphio/properties.h>

#include <highfive/H5File.hpp>

namespace morphio {
namespace readers {
namespace h5 {
Property::Properties load(const std::string& uri);
Property::Properties load(const HighFive::Group& group);

class MorphologyHDF5
{
  public:
    MorphologyHDF5(const HighFive::Group& group);
    virtual ~MorphologyHDF5() = default;
    Property::Properties load();

  private:
    void _checkVersion(const std::string& source);
    void _readMetadata(const std::string& source);
    void _readPoints(int);
    int _readSections();
    void _readPerimeters(int);
    void _readMitochondria();
    void _readEndoplasmicReticulum();
    void _readDendriticSpinePostSynapticDensity();

    template <typename T>
    void _read(const std::string& group,
               const std::string& _dataset,
               unsigned int expectedDimension,
               T& data);

    HighFive::Group _group;
    Property::Properties _properties;
    std::string _uri;
};
}  // namespace h5
}  // namespace readers
}  // namespace morphio
