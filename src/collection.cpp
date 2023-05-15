#include <morphio/collection.h>

#include "shared_utils.hpp"
#include <highfive/H5File.hpp>

#include "readers/morphologyHDF5.h"

namespace morphio {

class CollectionImpl
{
  public:
    virtual ~CollectionImpl() = default;

    virtual Morphology load(const std::string& morph_name, unsigned int options) const = 0;
    virtual mut::Morphology load_mut(const std::string& morph_name, unsigned int options) const = 0;
};

namespace detail {
template <class Derived>
class CollectionImpl: public morphio::CollectionImpl
{
    // The purpose of this class is to implement the two separate
    // function in terms of a single templated method `load_impl`.
  public:
    morphio::Morphology load(const std::string& morph_name, unsigned int options) const override {
        const auto& derived = static_cast<const Derived&>(*this);
        return derived.template load_impl<Morphology>(morph_name, options);
    }

    morphio::mut::Morphology load_mut(const std::string& morph_name,
                                      unsigned int options) const override {
        const auto& derived = static_cast<const Derived&>(*this);
        return derived.template load_impl<mut::Morphology>(morph_name, options);
    }
};
}  // namespace detail

class DirectoryCollection: public morphio::detail::CollectionImpl<DirectoryCollection>
{
  public:
    DirectoryCollection(std::string collection_path, std::vector<std::string> extensions)
        : _dirname(std::move(collection_path))
        , _extensions(std::move(extensions)) {}

  protected:
    friend morphio::detail::CollectionImpl<DirectoryCollection>;

    template <class M>
    M load_impl(const std::string& morph_name, unsigned int options) const {
        return M(morphology_path(morph_name), options);
    }

    std::string morphology_path(const std::string& morph_name) const {
        for (const auto& ext : _extensions) {
            auto path = morphio::join_path(_dirname, morph_name + ext);
            if (morphio::is_regular_file(path)) {
                return path;
            }
        }

        throw MorphioError("Morpholgy '" + morph_name + "' not found in: " + _dirname);
    }

  private:
    std::string _dirname;
    std::vector<std::string> _extensions;
};

class HDF5ContainerCollection: public morphio::detail::CollectionImpl<HDF5ContainerCollection>
{
  public:
    /**
     * Create the collection from an HDF5 container.
     *
     * This provides a way for C++ applications to inject specifics about how
     * to open the container.
     */
    HDF5ContainerCollection(HighFive::File file)
        : _file(std::move(file)) {}

    /**
     * Create the collection from a path.
     */
    HDF5ContainerCollection(const std::string& collection_path)
        : HDF5ContainerCollection(default_open_file(collection_path)) {}

    HDF5ContainerCollection(HDF5ContainerCollection&&) = delete;
    HDF5ContainerCollection(const HDF5ContainerCollection&) = delete;

    HDF5ContainerCollection& operator=(const HDF5ContainerCollection&) = delete;
    HDF5ContainerCollection& operator=(HDF5ContainerCollection&&) = delete;

  protected:
    friend morphio::detail::CollectionImpl<HDF5ContainerCollection>;

    template <class M>
    M load_impl(const std::string& morph_name, unsigned int options) const {
        std::lock_guard<std::recursive_mutex> lock(morphio::readers::h5::global_hdf5_mutex());
        return M(_file.getGroup(morph_name), options);
    }

  protected:
    static HighFive::File default_open_file(const std::string& container_path) {
        std::lock_guard<std::recursive_mutex> lock(morphio::readers::h5::global_hdf5_mutex());
        return HighFive::File(container_path, HighFive::File::ReadOnly);
    }

  private:
    HighFive::File _file;
};

namespace detail {
static std::shared_ptr<morphio::CollectionImpl> open_collection(
    std::string collection_path, std::vector<std::string> extensions) {
    if (morphio::is_directory(collection_path)) {
        // Triggers loading SWC, ASC, H5, etc. morphologies that are stored as
        // separate files in one directory.
        return std::make_shared<DirectoryCollection>(std::move(collection_path),
                                                     std::move(extensions));
    }

    if (morphio::is_regular_file(collection_path)) {
        // Prepare to load from containers.
        return std::make_shared<HDF5ContainerCollection>(std::move(collection_path));
    }

    throw std::invalid_argument("Invalid path: " + collection_path);
}
}  // namespace detail

Collection::Collection(std::shared_ptr<CollectionImpl> collection)
    : _collection(std::move(collection)) {
    if (_collection == nullptr) {
        throw std::invalid_argument("Can't construct a collection from a nullptr.");
    }
}

Collection::Collection(std::string collection_path, std::vector<std::string> extensions)
    : Collection(detail::open_collection(std::move(collection_path), std::move(extensions))) {}


template <class M>
typename std::enable_if<std::is_same<M, Morphology>::value, M>::type Collection::load(
    const std::string& morph_name, unsigned int options) const {
    if (_collection != nullptr) {
        return _collection->load(morph_name, options);
    }

    throw std::runtime_error("The collection has been closed.");
}

template <class M>
typename std::enable_if<std::is_same<M, mut::Morphology>::value, M>::type Collection::load(
    const std::string& morph_name, unsigned int options) const {
    if (_collection != nullptr) {
        return _collection->load_mut(morph_name, options);
    }

    throw std::runtime_error("The collection has been closed.");
}


template typename std::enable_if<std::is_same<mut::Morphology, mut::Morphology>::value,
                                 mut::Morphology>::type
Collection::load<mut::Morphology>(const std::string& morph_name, unsigned int options) const;

template typename std::enable_if<std::is_same<Morphology, Morphology>::value, Morphology>::type
Collection::load<Morphology>(const std::string& morph_name, unsigned int options) const;

void Collection::close() {
    _collection = nullptr;
}

}  // namespace morphio
