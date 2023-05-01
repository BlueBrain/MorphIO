#include <catch2/catch.hpp>

#include <morphio/collection.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>

#include <filesystem>
namespace fs = std::filesystem;

template <class T>
auto maybe_dereference(const std::shared_ptr<T>& obj) {
    return *obj;
}

template <class T>
auto maybe_dereference(const T& obj) {
    return obj;
}

template <class M>
std::string mutability_label();

template <>
std::string mutability_label<morphio::Morphology>() {
    return "immutable";
}

template <>
std::string mutability_label<morphio::mut::Morphology>() {
    return "mutable";
}

template <class M>
void check_collection_vs_single_file(const morphio::Collection& collection,
                                     const std::string& collection_dir,
                                     const std::string& morph_name) {
    auto expected = M(fs::path(collection_dir) / (morph_name + ".h5"));
    auto actual = collection.load<M>(morph_name);

    auto actual_soma = maybe_dereference(actual.soma());
    auto expected_soma = maybe_dereference(expected.soma());

    REQUIRE(actual_soma.points().size() == expected_soma.points().size());
    REQUIRE(actual.sections().size() == expected.sections().size());
}

template <class M>
void check_directory_vs_single_file(const std::string& collection_dir,
                                    const std::string& morph_name) {
    SECTION("directory: " + mutability_label<M>() + ": " + morph_name) {
        auto collection = morphio::Collection(collection_dir);
        check_collection_vs_single_file<M>(collection, collection_dir, morph_name);
    }
}

template <class M>
void check_container_vs_single_file(const std::string& collection_dir,
                                    const std::string& morph_name) {
    SECTION("merged: " + mutability_label<M>() + ": " + morph_name) {
        auto collection = morphio::Collection(fs::path(collection_dir) / "merged.h5");
        check_collection_vs_single_file<M>(collection, collection_dir, morph_name);
    }
}

template <class M>
void check_vs_single_file(const std::string& collection_dir, const std::string& morph_name) {
    check_directory_vs_single_file<M>(collection_dir, morph_name);
    check_container_vs_single_file<M>(collection_dir, morph_name);
}

TEST_CASE("Collection", "[collection]") {
    auto morphology_names =
        std::vector<std::string>{"simple", "glia", "mitochondria", "endoplasmic-reticulum"};

    for (const auto& morph_name : morphology_names) {
        check_vs_single_file<morphio::Morphology>("data/h5/v1", morph_name);
        check_vs_single_file<morphio::mut::Morphology>("data/h5/v1", morph_name);
    }
}
