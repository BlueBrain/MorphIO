#include <morphio/collection.h>

#include "shared_utils.hpp"
#include <highfive/H5File.hpp>

namespace morphio {

namespace detail {
template <class Derived>
class CollectionImpl: public morphio::CollectionImpl
{
    // The purpose of this class is to implement the two separate
    // function in terms of a single templated method `load_impl`.
  public:
    morphio::Morphology load(const std::string& morph_name) const override {
        const auto& derived = static_cast<const Derived&>(*this);
        return derived.template load_impl<Morphology>(morph_name);
    }

    morphio::mut::Morphology load_mut(const std::string& morph_name) const override {
        const auto& derived = static_cast<const Derived&>(*this);
        return derived.template load_impl<mut::Morphology>(morph_name);
    }
};
}  // namespace detail

class DirectoryCollection: public morphio::detail::CollectionImpl<DirectoryCollection>
{
  public:
    DirectoryCollection(std::string collection_path, std::vector<std::string> extensions)
        : _dirname(std::move(collection_path))
        , _extensions(extensions) {}

  protected:
    friend morphio::detail::CollectionImpl<DirectoryCollection>;

    template <class M>
    M load_impl(const std::string& morph_name) const {
        return M(morphology_path(morph_name));
    }

    std::string morphology_path(const std::string& morph_name) const {
        for (const auto& ext : _extensions) {
            auto path = morphio::join_path(_dirname, morph_name + "." + ext);
            if (morphio::is_regular_file(path)) {
                return path;
            }
        }

        throw std::runtime_error("Morpholgy '" + morph_name + "' not found in: " + _dirname);
    }

  private:
    std::string _dirname;
    std::vector<std::string> _extensions;
};

class ContainerCollection: public morphio::detail::CollectionImpl<ContainerCollection>
{
  public:
    /**
     * Create the collection from an HDF5 container.
     *
     * This provides a way for C++ applications to inject specifics about how
     * to open the container.
     */
    ContainerCollection(const HighFive::File& file)
        : _file(file) {}

    /**
     * Create the collection from a path.
     */
    ContainerCollection(const std::string& collection_path)
        : ContainerCollection(default_open_file(collection_path)) {}

  protected:
    friend morphio::detail::CollectionImpl<ContainerCollection>;

    template <class M>
    M load_impl(const std::string& morph_name) const {
        return M(_file.getGroup(morph_name));
    }

  protected:
    static HighFive::File default_open_file(const std::string& container_path) {
        return HighFive::File(container_path, HighFive::File::ReadOnly);
    }

  private:
    HighFive::File _file;
};

namespace detail {
std::shared_ptr<morphio::CollectionImpl> open_collection(std::string collection_path,
                                                         std::vector<std::string> extensions) {
    if (morphio::is_directory(collection_path)) {
        // Triggers loading SWC, ASC, H5, etc. morphologies that are stored as
        // separate files in one directory.
        return std::make_shared<DirectoryCollection>(std::move(collection_path),
                                                     std::move(extensions));
    }

    if (morphio::is_regular_file(collection_path)) {
        // Prepare to load from containers.
        return std::make_shared<ContainerCollection>(collection_path);
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

void Collection::close() {
    _collection = nullptr;
}

}  // namespace morphio
