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
                                     const std::string& morph_name,
                                     const std::string& reference_path) {
    auto expected = M(reference_path);
    auto actual = collection.load<M>(morph_name);

    auto actual_soma = maybe_dereference(actual.soma());
    auto expected_soma = maybe_dereference(expected.soma());

    REQUIRE(actual_soma.points().size() == expected_soma.points().size());
    REQUIRE(actual.sections().size() == expected.sections().size());
}

template <class M>
void check_directory_vs_single_file(const std::string& collection_dir,
                                    const std::string& morph_name,
                                    const std::string& reference_path) {
    SECTION("directory: " + mutability_label<M>() + ": " + morph_name) {
        auto collection = morphio::Collection(collection_dir);
        check_collection_vs_single_file<M>(collection, morph_name, reference_path);
    }
}

template <class M>
void check_container_vs_single_file(const std::string& collection_dir,
                                    const std::string& container_name,
                                    const std::string& morph_name,
                                    const std::string& reference_path) {
    SECTION(container_name + ": " + mutability_label<M>() + ": " + morph_name) {
        auto collection = morphio::Collection(fs::path(collection_dir) / container_name);
        check_collection_vs_single_file<M>(collection, morph_name, reference_path);
    }
}

template <class M>
void check_vs_single_file(const std::string& collection_dir,
                          const std::string& morph_name,
                          const std::string& reference_path) {
    check_directory_vs_single_file<M>(collection_dir, morph_name, reference_path);
    check_container_vs_single_file<M>(collection_dir, "merged.h5", morph_name, reference_path);
    check_container_vs_single_file<M>(collection_dir, "unified.h5", morph_name, reference_path);
}

TEST_CASE("Collection", "[collection]") {
    auto morphology_names =
        std::vector<std::string>{"simple", "glia", "mitochondria", "endoplasmic-reticulum"};
    auto collection_dir = std::string("data/h5/v1");

    for (const auto& morph_name : morphology_names) {
        auto reference_path = fs::path(collection_dir) / (morph_name + ".h5");
        check_vs_single_file<morphio::Morphology>(collection_dir,
                                                  morph_name,
                                                  reference_path.string());
        check_vs_single_file<morphio::mut::Morphology>(collection_dir,
                                                       morph_name,
                                                       reference_path.string());
    }
}

TEST_CASE("CollectionMissingExtensions", "[collection]") {
    auto collection_dir = std::string("data");

    SECTION("missing .h5") {
        auto collection = morphio::Collection(collection_dir);
        auto morph_name = std::string("simple");
        auto reference_path = fs::path(collection_dir) / (morph_name + ".asc");
        check_collection_vs_single_file<morphio::Morphology>(collection,
                                                             morph_name,
                                                             reference_path.string());
    }

    SECTION("missing .h5 and .asc") {
        auto collection = morphio::Collection(collection_dir);
        auto morph_name = std::string("soma_cylinders");
        auto reference_path = fs::path(collection_dir) / (morph_name + ".swc");
        check_collection_vs_single_file<morphio::Morphology>(collection,
                                                             morph_name,
                                                             reference_path.string());
    }

    SECTION("custom extensions") {
        auto collection = morphio::Collection(collection_dir, {".h5", ".asc"});
        auto morph_name = std::string("soma_cylinders");
        auto reference_path = fs::path(collection_dir) / (morph_name + ".swc");
        CHECK_THROWS(check_collection_vs_single_file<morphio::Morphology>(collection,
                                                                          morph_name,
                                                                          reference_path.string()));
    }
}
