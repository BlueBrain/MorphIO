#include <catch2/catch.hpp>

#include <morphio/collection.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>

#include <algorithm>
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
                                    const std::string& morph_name,
                                    const std::string& reference_path) {
    SECTION("merged: " + mutability_label<M>() + ": " + morph_name) {
        auto collection = morphio::Collection(fs::path(collection_dir) / "merged.h5");
        check_collection_vs_single_file<M>(collection, morph_name, reference_path);
    }
}

template <class M>
void check_vs_single_file(const std::string& collection_dir,
                          const std::string& morph_name,
                          const std::string& reference_path) {
    check_directory_vs_single_file<M>(collection_dir, morph_name, reference_path);
    check_container_vs_single_file<M>(collection_dir, morph_name, reference_path);
}

TEST_CASE("Collection", "[collection]") {
    auto morphology_names = std::vector<std::string>{
        "simple", "glia", "mitochondria", "endoplasmic-reticulum", "simple-dendritric-spine"};
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

static void check_loop_indices(std::vector<size_t>& loop_indices, size_t n) {
    REQUIRE(loop_indices.size() == n);
    std::sort(loop_indices.begin(), loop_indices.end());
    for (size_t i = 0; i < loop_indices.size(); ++i) {
        REQUIRE(loop_indices[i] == i);
    }
}

static void check_collection_load_unordered(const std::string& collection_path) {
    morphio::Collection collection(collection_path);

    auto morphology_names = std::vector<std::string>{
        "simple", "glia", "mitochondria", "endoplasmic-reticulum", "simple-dendritric-spine"};

    SECTION("modern") {
        std::vector<size_t> loop_indices;
        for (auto [k, morph] : collection.load_unordered<morphio::Morphology>(morphology_names)) {
            loop_indices.push_back(k);
        }
        check_loop_indices(loop_indices, morphology_names.size());
    }

    SECTION("classical") {
        std::vector<size_t> loop_indices;
        auto unordered_access = collection.load_unordered<morphio::Morphology>(morphology_names);
        for (auto it = unordered_access.begin(); it != unordered_access.end(); ++it) {
            auto [k, morph] = *it;
            loop_indices.push_back(k);
        }
        check_loop_indices(loop_indices, morphology_names.size());
    }
}

TEST_CASE("Collection::load_unordered directory", "[collection]") {
    check_collection_load_unordered("data/h5/v1");
}

TEST_CASE("Collection::load_unordered merged", "[collection]") {
    check_collection_load_unordered("data/h5/v1/merged.h5");
}

static void check_collection_argsort(const std::string& collection_path) {
    morphio::Collection collection(collection_path);

    auto morphology_names = std::vector<std::string>{"simple",
                                                     "glia",
                                                     "endoplasmic-reticulum",
                                                     "simple-dendritric-spine"};

    auto loop_indices = collection.argsort(morphology_names);
    check_loop_indices(loop_indices, morphology_names.size());
}

TEST_CASE("Collection::argsort directory", "[collection]") {
    check_collection_argsort("data/h5/v1");
}

TEST_CASE("Collection::argsort merged", "[collection]") {
    check_collection_argsort("data/h5/v1/merged.h5");
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

TEST_CASE("LoadUnordered::Iterator", "[collection]") {
    auto collection = morphio::Collection("data/h5/v1/merged.h5");
    auto morphology_names = std::vector<std::string>{
        "simple", "glia", "mitochondria", "endoplasmic-reticulum", "simple-dendritric-spine"};

    auto loader = collection.load_unordered<morphio::Morphology>(morphology_names);
    auto begin = loader.begin();
    auto k_begin = (*begin).first;

    // Create a copy, and check they are at the same loop index.
    auto it = begin;
    REQUIRE((*it).first == (*begin).first);

    // Increment the copy, postfix:
    auto it2 = it++;
    REQUIRE((*it2).first == (*begin).first);
    REQUIRE((*it).first != k_begin);

    // Increment a copy, prefix:
    auto it3 = ++it2;
    REQUIRE((*it2).first == (*it).first);
    REQUIRE((*it2).first == (*it3).first);
    REQUIRE((*it).first != k_begin);

    // Now check that nothing incremented the original iterator `begin`.
    REQUIRE((*begin).first == k_begin);

    // Once more using assignment, not copy-construction:
    it = begin;
    REQUIRE((*begin).first == k_begin);
    REQUIRE((*++it).first != k_begin);
}
