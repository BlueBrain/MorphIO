#pragma once

#include <memory>
#include <string>

#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>

namespace morphio {

class CollectionImpl;

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
    typename std::enable_if<std::is_same<M, Morphology>::value, M>::type load(
        const std::string& morph_name) const;

    /**
     * Load the morphology as a mutable morphology.
     */
    template <class M>
    typename std::enable_if<std::is_same<M, mut::Morphology>::value, M>::type load(
        const std::string& morph_name) const;

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

extern template mut::Morphology Collection::load<mut::Morphology>(
    const std::string& morph_name) const;

extern template Morphology Collection::load<Morphology>(const std::string& morph_name) const;

}  // namespace morphio
