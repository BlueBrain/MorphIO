/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>
#include <string>

#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>

namespace morphio {

class CollectionImpl;

template <class M>
class LoadUnordered;

/**
 * Enable if `T` is a immutable morphology.
 */
template <class T, class U = void>
struct enable_if_immutable: public std::enable_if<std::is_same<T, Morphology>::value, U> {};

/**
 * Enable if `T` is a mutable morphology.
 */
template <class T, class U = void>
struct enable_if_mutable: public std::enable_if<std::is_same<T, mut::Morphology>::value, U> {};

class Collection
{
  public:
    Collection(std::shared_ptr<CollectionImpl> collection);

    /**
     * Create a collection from the given path.
     *
     * If `collection_path` points to an HDF5 file, then that file
     * must be a container. Otherwise the `collection_path` should
     * point to the directory containing the morphology files.
     *
     * If the collection path is a directory, the extension of the morphology
     * file must be guessed. The optional argument `extensions` specifies which
     * and in which order the morphologies are searched.
     */
    Collection(std::string collection_path,
               std::vector<std::string> extensions =
                   std::vector<std::string>{".h5", ".H5", ".asc", ".ASC", ".swc", ".SWC"});

    /**
     * Load the morphology as an immutable morphology.
     */
    template <class M>
    typename enable_if_mutable<M, M>::type load(const std::string& morph_name,
                                                unsigned int options = NO_MODIFIER) const;

    /**
     * Load the morphology as a mutable morphology.
     */
    template <class M>
    typename enable_if_immutable<M, M>::type load(const std::string& morph_name,
                                                  unsigned int options = NO_MODIFIER) const;

    /**
     * Returns an iterable of loop index, morphology pairs.
     *
     * See `LoadUnordered` for details.
     */
    template <class M>
    LoadUnordered<M> load_unordered(std::vector<std::string> morphology_names,
                                    unsigned int options = NO_MODIFIER) const;

    /**
     * Returns the reordered loop indices.
     *
     * This is the suggested order in which one should load the morphologies to
     * minimize seeking within the file.
     *
     * Note: This API is 'experimental', meaning it might change in the future.
     */
    std::vector<size_t> argsort(const std::vector<std::string>& morphology_names) const;

    /**
     * Close the collection.
     *
     * Note that `morphio::Collection` uses RAII. Therefore, the usual
     * scoping rules should suffice. However, there are cases where one
     * wants to close the collection explicitly, causing all resources held
     * by the collection to be release. In the case of containers it ensures
     * that the file is closed.
     *
     * The object should not be used after calling `close`.
     */
    void close();

  private:
    std::shared_ptr<CollectionImpl> _collection;
};

class LoadUnorderedImpl;

/**
 * An iterable of loop index and morphologies.
 *
 * When reading from containers, the order in which morphologies are read can
 * have a large impact on the overall time to load those morphologies.
 *
 * This iterator provides means of reordering loops to optimize the access
 * pattern. Loops such as the following
 *
 *     for(size_t k = 0; k < morphology_names.size; ++k) {
 *       auto morph = collection.load<M>(morphology_names[k]);
 *       f(k, morph);
 *     }
 *
 * can be replaced with
 *
 *     for(auto [k, morph] : collection.load_unordered<M>(morphology_names)) {
 *       assert(collection.load<M>(morphology_names[k]) == morph);
 *       f(k, morph);
 *     }
 *
 * The order in which the morphologies are returned in unspecified, but the
 * loop index `k` can be used to retrieve the correct state corresponding to
 * iteration `k` of the original loop.
 *
 * Note, that it is safe for an `LoadUnordered` object to outlive its
 * `collection`. Internally a shallow copy of the original `collection` is
 * stored inside of and kept alive for the life time of the `LoadUnordered`
 * object.
 *
 * Note: This API is 'experimental', meaning it might change in the future.
 */
template <class M>
class LoadUnordered
{
  protected:
    class Iterator
    {
      public:
        Iterator(std::shared_ptr<LoadUnorderedImpl> load_unordered_impl, size_t k);

        template <class U = M>
        typename enable_if_immutable<U, std::pair<size_t, M>>::type operator*() const;

        template <class U = M>
        typename enable_if_mutable<U, std::pair<size_t, M>>::type operator*() const;

        Iterator& operator++();
        Iterator operator++(int);

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;

      private:
        size_t _k;
        std::shared_ptr<LoadUnorderedImpl> _load_unordered_impl;
    };

  public:
    LoadUnordered(std::shared_ptr<LoadUnorderedImpl> load_unordered_impl);

    Iterator begin() const;
    Iterator end() const;

  protected:
    std::shared_ptr<LoadUnorderedImpl> _load_unordered_impl;
};

extern template class LoadUnordered<Morphology>;
extern template class LoadUnordered<mut::Morphology>;

extern template class LoadUnordered<Morphology>::Iterator;
extern template class LoadUnordered<mut::Morphology>::Iterator;

extern template typename enable_if_immutable<Morphology, std::pair<size_t, Morphology>>::type
    LoadUnordered<Morphology>::Iterator::operator*<Morphology>() const;

extern template
    typename enable_if_mutable<mut::Morphology, std::pair<size_t, mut::Morphology>>::type
        LoadUnordered<mut::Morphology>::Iterator::operator*<mut::Morphology>() const;

extern template typename enable_if_mutable<mut::Morphology, mut::Morphology>::type
Collection::load<mut::Morphology>(const std::string& morph_name, unsigned int options) const;

extern template typename enable_if_immutable<Morphology, Morphology>::type
Collection::load<Morphology>(const std::string& morph_name, unsigned int options) const;

extern template LoadUnordered<Morphology> Collection::load_unordered<Morphology>(
    std::vector<std::string> morphology_names, unsigned int options) const;

extern template LoadUnordered<mut::Morphology> Collection::load_unordered<mut::Morphology>(
    std::vector<std::string> morphology_names, unsigned int options) const;

}  // namespace morphio
