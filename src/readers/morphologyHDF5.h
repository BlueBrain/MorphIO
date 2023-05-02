#pragma once
#include <string>  // std::string
#include <mutex>

#include <morphio/properties.h>

#include <highfive/H5File.hpp>

namespace morphio {
namespace readers {
namespace h5 {
Property::Properties load(const std::string& uri);
Property::Properties load(const HighFive::Group& group);

namespace detail {
/**
 * Provides the API for reading datasets from HDF5 files.
 *
 * The CRTP requirements are that `Derived` has a method
 * `read_impl` with the same signature as `read`.
 */
template <class Derived>
class DataSetReader
{
  public:
    /**
     * Read a dataset from an HDF5 file.
     *
     * Arguments:
     *   @param groupName The datasets of a morphology are organized in
     *   subgroups, e.g. `organelled`. The `groupName` refers to the group
     *   name relative to the morphology group, which is the root group for
     *   single files or some other group for merged containers. Note, that
     *   `groupName == ""` for datasets stored in the morphology directly
     *   group, e.g. `points` and `structure`.
     *
     *   @param dataSetName The name of the dataset to read, relative to
     *   `groupName`.
     *
     *   @param expectedDimensions Enables checking if the dimension of the
     *   dataset match expectations. Entries with `size_t(-1)` will be
     *   skipped during validation.
     *
     *   @param out The output will be resized if needed.
     */
    template <class T>
    void read(const std::string& groupName,
              const std::string& dataSetName,
              const std::vector<size_t> expectedDimensions,
              T& out) const;
};

/**
 * The implementation for reading from single HDF5 files or merged containers.
 */
class MergedReader: public DataSetReader<MergedReader>
{
  public:
    MergedReader(const HighFive::Group& group)
        : _group(group) {}

  protected:
    friend DataSetReader<MergedReader>;

    template <class T>
    void read_impl(const std::string& groupName,
                   const std::string& dataSetName,
                   const std::vector<size_t> expectedDimensions,
                   T& out) const;

  private:
    HighFive::Group _group;
};
}  // namespace detail

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

    detail::MergedReader _merged_reader;
};

inline std::recursive_mutex& global_hdf5_mutex() {
    static std::recursive_mutex _mutex;
    return _mutex;
}

}  // namespace h5
}  // namespace readers
}  // namespace morphio
