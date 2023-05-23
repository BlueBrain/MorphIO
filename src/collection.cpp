#include <morphio/collection.h>

#include "shared_utils.hpp"
#include <highfive/H5File.hpp>

#include "readers/morphologyHDF5.h"

namespace morphio {

class LoadUnorderedIteratorImpl
{
  public:
    virtual ~LoadUnorderedIteratorImpl() = default;

    virtual std::pair<size_t, Morphology> load() const = 0;
    virtual std::pair<size_t, mut::Morphology> load_mut() const = 0;

    virtual void advance() = 0;
    virtual bool operator==(const LoadUnorderedIteratorImpl& other) const = 0;
    bool operator!=(const LoadUnorderedIteratorImpl& other) const {
        return !((*this) == other);
    }
};

class LoadUnorderedImpl
{
  public:
    using Iterator = LoadUnorderedIteratorImpl;

    virtual ~LoadUnorderedImpl() = default;

    virtual std::shared_ptr<Iterator> begin_as_shared() const = 0;
    virtual std::shared_ptr<Iterator> end_as_shared() const = 0;
};

namespace detail {

/**
 *  Load morphologies in the specified order.
 *
 *  Given access to the `collection`, the morphology names and an ordered
 *  vectors of loop indices, one can implement simple optimized access pattern
 *  by simply loading the morphologies in the order the loop indices prescribe.
 *
 *  The above approach is implemented by this class.
 */
class LoadUnorderedFromLoopIndices: public LoadUnorderedImpl
{
  public:
    class Iterator: public LoadUnorderedImpl::Iterator
    {
      public:
        Iterator(size_t k, const LoadUnorderedFromLoopIndices& container)
            : _k(k)
            , _container(container) {}

        std::pair<size_t, Morphology> load() const override {
            return load_impl<Morphology>();
        }

        std::pair<size_t, mut::Morphology> load_mut() const override {
            return load_impl<mut::Morphology>();
        }

        void advance() override {
            ++_k;
        }

        bool operator==(const LoadUnorderedImpl::Iterator& any_other) const override {
            if (dynamic_cast<const Iterator*>(&any_other) != nullptr) {
                const Iterator& other = static_cast<const Iterator&>(any_other);
                return (*this)._k == other._k;
            }

            return false;
        }

      protected:
        template <class M>
        std::pair<size_t, M> load_impl() const {
            auto i = _container._loop_indices[_k];
            const auto& morph_name = _container._morphology_names[i];
            auto options = _container._options;
            auto m = _container._collection.template load<M>(morph_name, options);
            return {i, std::move(m)};
        }

      private:
        size_t _k;
        const LoadUnorderedFromLoopIndices& _container;
    };

    LoadUnorderedFromLoopIndices(Collection collection,
                                 std::vector<size_t> loop_indices,
                                 std::vector<std::string> morphology_names,
                                 unsigned int options)
        : _collection(std::move(collection))
        , _loop_indices(std::move(loop_indices))
        , _morphology_names(std::move(morphology_names))
        , _options(options) {}

    std::shared_ptr<LoadUnorderedImpl::Iterator> begin_as_shared() const override {
        return std::make_shared<Iterator>(0, *this);
    }

    std::shared_ptr<LoadUnorderedImpl::Iterator> end_as_shared() const override {
        return std::make_shared<Iterator>(_morphology_names.size(), *this);
    }

  private:
    Collection _collection;
    std::vector<size_t> _loop_indices;
    std::vector<std::string> _morphology_names;
    unsigned int _options;
};

}  // namespace detail


class CollectionImpl
{
  public:
    virtual ~CollectionImpl() = default;

    virtual Morphology load(const std::string& morph_name, unsigned int options) const = 0;

    virtual mut::Morphology load_mut(const std::string& morph_name, unsigned int options) const = 0;

    virtual std::shared_ptr<LoadUnorderedImpl> load_unordered(Collection collection,
                                                              std::vector<std::string> morph_name,
                                                              unsigned int options) const = 0;
};

namespace detail {
template <class Derived>
class CollectionImpl: public morphio::CollectionImpl
{
    // The purpose of this class is to implement the two separate
    // functions in terms of a single templated method `load_impl`.
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

        throw MorphioError("Morphology '" + morph_name + "' not found in: " + _dirname);
    }

    std::shared_ptr<LoadUnorderedImpl> load_unordered(Collection collection,
                                                      std::vector<std::string> morphology_names,
                                                      unsigned int options) const override {
        auto n_morphologies = morphology_names.size();
        std::vector<size_t> loop_indices(n_morphologies);
        for (size_t i = 0; i < n_morphologies; ++i) {
            loop_indices[i] = i;
        }

        return std::make_shared<detail::LoadUnorderedFromLoopIndices>(std::move(collection),
                                                                      std::move(loop_indices),
                                                                      std::move(morphology_names),
                                                                      options);
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

    std::shared_ptr<LoadUnorderedImpl> load_unordered(Collection collection,
                                                      std::vector<std::string> morphology_names,
                                                      unsigned int options) const override {
        auto n_morphologies = morphology_names.size();
        std::vector<hsize_t> offsets(n_morphologies);
        std::vector<size_t> loop_indices(n_morphologies);

        for (size_t i = 0; i < n_morphologies; ++i) {
            loop_indices[i] = i;

            const auto& morph_name = morphology_names[i];

            auto morph = _file.getGroup(morph_name.data());
            auto points = morph.getDataSet("points");

            auto dcpl = points.getCreatePropertyList();
            auto layout = H5Pget_layout(dcpl.getId());
            offsets[i] = layout == H5D_CONTIGUOUS ? points.getOffset() : size_t(-1);
        }

        std::sort(loop_indices.begin(), loop_indices.end(), [&offsets](size_t i, size_t j) {
            return offsets[i] < offsets[j];
        });

        return std::make_shared<detail::LoadUnorderedFromLoopIndices>(std::move(collection),
                                                                      std::move(loop_indices),
                                                                      std::move(morphology_names),
                                                                      options);
    }

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
typename enable_if_immutable<M, M>::type Collection::load(const std::string& morph_name,
                                                          unsigned int options) const {
    if (_collection != nullptr) {
        return _collection->load(morph_name, options);
    }

    throw std::runtime_error("The collection has been closed.");
}

template <class M>
typename enable_if_mutable<M, M>::type Collection::load(const std::string& morph_name,
                                                        unsigned int options) const {
    if (_collection != nullptr) {
        return _collection->load_mut(morph_name, options);
    }

    throw std::runtime_error("The collection has been closed.");
}

template mut::Morphology Collection::load<mut::Morphology>(const std::string& morph_name,
                                                           unsigned int options) const;

template Morphology Collection::load<Morphology>(const std::string& morph_name,
                                                 unsigned int options) const;


template <class M>
LoadUnordered<M> Collection::load_unordered(std::vector<std::string> morphology_names,
                                            unsigned int options) const {
    return LoadUnordered<M>(_collection->load_unordered(*this, morphology_names, options));
}

template LoadUnordered<mut::Morphology> Collection::load_unordered<mut::Morphology>(
    std::vector<std::string> morphology_names, unsigned int options) const;

template LoadUnordered<Morphology> Collection::load_unordered<Morphology>(
    std::vector<std::string> morphology_names, unsigned int options) const;


void Collection::close() {
    _collection = nullptr;
}

template <class M>
typename LoadUnordered<M>::Iterator LoadUnordered<M>::begin() const {
    return Iterator(_load_unordered->begin_as_shared());
}

template typename LoadUnordered<Morphology>::Iterator LoadUnordered<Morphology>::begin() const;

template typename LoadUnordered<mut::Morphology>::Iterator LoadUnordered<mut::Morphology>::begin()
    const;

template <class M>
typename LoadUnordered<M>::Iterator LoadUnordered<M>::end() const {
    return Iterator(_load_unordered->end_as_shared());
}

template typename LoadUnordered<Morphology>::Iterator LoadUnordered<Morphology>::end() const;

template typename LoadUnordered<mut::Morphology>::Iterator LoadUnordered<mut::Morphology>::end()
    const;

template <class M>
LoadUnordered<M>::LoadUnordered(std::shared_ptr<LoadUnorderedImpl> load_unordered)
    : _load_unordered(load_unordered) {}

template LoadUnordered<Morphology>::LoadUnordered(
    std::shared_ptr<LoadUnorderedImpl> load_unordered);

template LoadUnordered<mut::Morphology>::LoadUnordered(
    std::shared_ptr<LoadUnorderedImpl> load_unordered);

template <class M>
LoadUnordered<M>::Iterator::Iterator(std::shared_ptr<LoadUnorderedImpl::Iterator> it)
    : _it(std::move(it)) {}

template LoadUnordered<Morphology>::Iterator::Iterator(
    std::shared_ptr<LoadUnorderedImpl::Iterator> it);

template LoadUnordered<mut::Morphology>::Iterator::Iterator(
    std::shared_ptr<LoadUnorderedImpl::Iterator> it);


template <class M>
bool LoadUnordered<M>::Iterator::operator==(const LoadUnordered::Iterator& other) const {
    return *(*this)._it == *other._it;
}

template bool LoadUnordered<Morphology>::Iterator::operator==(
    const LoadUnordered::Iterator& other) const;

template bool LoadUnordered<mut::Morphology>::Iterator::operator==(
    const LoadUnordered::Iterator& other) const;

template <class M>
bool LoadUnordered<M>::Iterator::operator!=(const LoadUnordered::Iterator& other) const {
    return !((*this) == other);
}

template bool LoadUnordered<Morphology>::Iterator::operator!=(
    const LoadUnordered::Iterator& other) const;

template bool LoadUnordered<mut::Morphology>::Iterator::operator!=(
    const LoadUnordered::Iterator& other) const;

template <class M>
void LoadUnordered<M>::Iterator::operator++() const {
    _it->advance();
}

template void LoadUnordered<Morphology>::Iterator::operator++() const;

template void LoadUnordered<mut::Morphology>::Iterator::operator++() const;


template <class M>
template <class U>
typename enable_if_mutable<U, std::pair<size_t, M>>::type LoadUnordered<M>::Iterator::operator*()
    const {
    return _it->load_mut();
}

template <class M>
template <class U>
typename enable_if_immutable<U, std::pair<size_t, M>>::type LoadUnordered<M>::Iterator::operator*()
    const {
    return _it->load();
}

template typename enable_if_mutable<mut::Morphology, std::pair<size_t, mut::Morphology>>::type
    LoadUnordered<mut::Morphology>::Iterator::operator*<mut::Morphology>() const;

template typename enable_if_immutable<Morphology, std::pair<size_t, Morphology>>::type
    LoadUnordered<Morphology>::Iterator::operator*<Morphology>() const;


}  // namespace morphio
