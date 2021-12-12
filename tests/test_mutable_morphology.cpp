#include "contrib/catch.hpp"

#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>

#include <filesystem>
namespace fs = std::filesystem;

TEST_CASE("is_heterogeneous", "[mutableMorphology]") {
    auto morph = morphio::mut::Morphology("data/simple-heterogeneous-neurite.swc");

    /** The morphology consists of two trees, with one bifurcation each. The root
     * sections had a different type than their respective children.
     */
    for (const auto& root_section : morph.rootSections()) {

        // We expect the root section to be heterogeneous downstream because
        // of their children of different type and homogeneous upstream because
        // there are no other sections.
        REQUIRE(root_section->is_heterogeneous(true)); // downstream = true
        REQUIRE(not root_section->is_heterogeneous(false)); // downstream = false

        // We expect the two children for each root section, which have a different
        // type to be homogeneous downstream as leaves and inhomogeneous upstream because
        // of the root section parent of different type.
        for (const auto& section: root_section->children()) {
            REQUIRE(not section->is_heterogeneous(true)); // downstream = true
            REQUIRE(section->is_heterogeneous(false)); // downstream = false
        }
    }
}


TEST_CASE("RemoveRootsection", "[mutableMorphology]") {
    // this test verifies we can delete a root section with recursive at false from a morphology
    // correctly. This is a special case where the root section as a single point and 2 child
    // sections. The number of sections is small enough to trigger a resize on _rootSections.
    morphio::mut::Morphology morph("data/single_point_root.asc");
    for (const auto& rootSection : morph.rootSections()) {
        if (rootSection->points().size() == 1) {
            morph.deleteSection(rootSection, false);
        }
    }
    REQUIRE(morph.rootSections().size() == 2);
}

TEST_CASE("RemoveUnifurcation", "[mutableMorphology]") {
    morphio::mut::Morphology morph("data/nested_single_children.asc");
    REQUIRE(morph.rootSections().size() == 1);
    REQUIRE(morph.rootSections()[0]->points().size() == 2);
    morph.removeUnifurcations();
    REQUIRE(morph.rootSections()[0]->points().size() == 5);
}

TEST_CASE("mutableConnectivity", "[mutableMorphology]") {
    morphio::mut::Morphology morph("data/simple.asc");
    std::unordered_map<int, std::vector<unsigned int>> expectedConnectivity = {{-1, {0, 3}},
                                                                               {0, {1, 2}},
                                                                               {3, {4, 5}}};

    REQUIRE(morph.connectivity() == expectedConnectivity);
}


TEST_CASE("writing", "[mutableMorphology]") {
    morphio::mut::Morphology morph("data/simple.asc");
    auto tmpDirectory = std::filesystem::temp_directory_path() / "test_mutable_morphology.cpp";
    std::filesystem::create_directories(tmpDirectory);
    morph.write(tmpDirectory / "simple.asc");
    morph.write(tmpDirectory / "simple.h5");
    morph.write(tmpDirectory / "simple.swc");
    morphio::Morphology savedMorphAsc(tmpDirectory / "simple.asc");
    morphio::Morphology savedMorphH5(tmpDirectory / "simple.h5");
    morphio::Morphology savedMorphSwc(tmpDirectory / "simple.swc");
    REQUIRE(savedMorphAsc.rootSections().size() == 2);
    REQUIRE(savedMorphH5.rootSections().size() == 2);
    REQUIRE(savedMorphSwc.rootSections().size() == 2);
    fs::remove_all(tmpDirectory);
}
