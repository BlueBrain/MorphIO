/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <catch2/catch.hpp>

#include <morphio/enums.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>

#include <filesystem>
namespace fs = std::filesystem;

TEST_CASE("isHeterogeneous", "[mutableMorphology]") {
    auto morph = morphio::mut::Morphology("data/simple-heterogeneous-neurite.swc");

    /** The morphology consists of two trees, with one bifurcation each. The root
     * sections had a different type than their respective children.
     */
    for (const auto& root_section : morph.rootSections()) {
        // We expect the root section to be heterogeneous downstream because
        // of their children of different type and homogeneous upstream because
        // there are no other sections.
        REQUIRE(root_section->isHeterogeneous(true));       // downstream = true
        REQUIRE(not root_section->isHeterogeneous(false));  // downstream = false

        // We expect the two children for each root section, which have a different
        // type to be homogeneous downstream as leaves and inhomogeneous upstream because
        // of the root section parent of different type.
        for (const auto& section : root_section->children()) {
            REQUIRE(not section->isHeterogeneous(true));  // downstream = true
            REQUIRE(section->isHeterogeneous(false));     // downstream = false
        }
    }
}

TEST_CASE("hasSameShape", "[mutableMorphology]") {
    {
        std::string path = "data/simple.asc";
        auto morph0 = morphio::mut::Morphology(path);
        auto morph1 = morphio::mut::Morphology(path);
        REQUIRE(morph0.rootSections()[0]->hasSameShape(*morph1.rootSections()[0]));
        REQUIRE(!morph0.rootSections()[0]->hasSameShape(*morph1.rootSections()[1]));
    }
    {
        std::string path = "data/h5/v1/mitochondria.h5";
        auto morph0 = morphio::mut::Morphology(path);
        auto morph1 = morphio::mut::Morphology(path);
        REQUIRE(morph0.mitochondria().rootSections()[0]->hasSameShape(
            *morph1.mitochondria().rootSections()[0]));
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
    auto tmpDirectory = std::filesystem::temp_directory_path() / "test_mutable_morphology.cpp";
    std::filesystem::create_directories(tmpDirectory);

    {
        morphio::mut::Morphology morph("data/simple.asc");
        morph.write(tmpDirectory / "simple.asc");
        morph.write(tmpDirectory / "simple.h5");

        morphio::Morphology savedMorphAsc(tmpDirectory / "simple.asc");
        morphio::Morphology savedMorphH5(tmpDirectory / "simple.h5");

        REQUIRE(savedMorphAsc.rootSections().size() == 2);
        REQUIRE(savedMorphH5.rootSections().size() == 2);

        // TODO: should turn this on when we are throwing errors on soma creation
        // swc should raise when writing a contour soma
        // CHECK_THROWS_AS(morph.write(tmpDirectory / "simple.swc"), morphio::WriterError);
    }

    {
        {
            morphio::mut::Morphology morph("data/simple.swc");
            morph.write(tmpDirectory / "simple.swc");
            morphio::Morphology savedMorphSwc(tmpDirectory / "simple.swc");
            REQUIRE(savedMorphSwc.rootSections().size() == 2);
            REQUIRE(savedMorphSwc.soma().type() == morphio::SomaType::SOMA_SINGLE_POINT);

            // TODO: should turn this on when we are throwing errors on soma creation
            // asc/h5 should raise when writing a non-contour soma
            // CHECK_THROWS_AS(morph.write(tmpDirectory / "simple.asc"), morphio::WriterError);
            // CHECK_THROWS_AS(morph.write(tmpDirectory / "simple.h5"), morphio::WriterError);
        }

        {
            morphio::mut::Morphology morph("data/simple.swc");
            morph.soma()->diameters().clear();
            CHECK_THROWS_AS(morph.write(tmpDirectory / "diameter-sample-mismatch.swc"),
                            morphio::WriterError);
        }

        /* { */
        /*     morphio::mut::Morphology morph("data/simple.swc"); */
        /*     for (const auto& rootSection : morph.rootSections()) { */
        /*         morph.deleteSection(rootSection, true); */
        /*     } */
        /*     CHECK_THROWS_AS(morph.write(tmpDirectory / "diameter-sample-mismatch.swc"),
         * morphio::WriterError); */
        /* } */

        {  // Can't write empty morphologies
            morphio::mut::Morphology morph;
            CHECK_THROWS_AS(morph.write(tmpDirectory / "empty.swc"), morphio::WriterError);
        }

        {  // Can write morph with no soma
            morphio::mut::Morphology morph;
            auto pt = morphio::Property::PointLevel({{0, 0, 0}, {1, 1, 1}}, {1, 1}, {});
            morph.appendRootSection(pt, morphio::SectionType::SECTION_AXON);
            morph.write(tmpDirectory / "no-soma.swc");
        }

        {  // Can write morph with SOMA_UNDEFINED/SOMA_SIMPLE_CONTOUR for now
            morphio::mut::Morphology morph;
            auto soma = morph.soma();
            soma->points() = {{1, 1, 1}};
            soma->diameters() = {1};

            soma->type() = morphio::SOMA_UNDEFINED;
            morph.write(tmpDirectory / "undefined-soma.swc");

            soma->type() = morphio::SOMA_SIMPLE_CONTOUR;
            morph.write(tmpDirectory / "simple-contour-soma.swc");
        }

        {  // should be a single point soma
            morphio::mut::Morphology morph;
            auto soma = morph.soma();
            soma->points() = {{1, 1, 1}, {2, 2, 2}};
            soma->diameters() = {1, 2};
            soma->type() = morphio::SOMA_SINGLE_POINT;
            CHECK_THROWS_AS(morph.write(tmpDirectory / "fail.swc"), morphio::WriterError);
        }

        {
            morphio::mut::Morphology morph("data/three_point_soma.swc");
            morph.write(tmpDirectory / "three_point_soma.swc");
            morphio::Morphology savedMorphSwc(tmpDirectory / "three_point_soma.swc");
            REQUIRE(savedMorphSwc.soma().type() ==
                    morphio::SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS);

            std::vector<morphio::Point>& points = morph.soma()->points();
            points[0][0] = 100;
            // this only prints to stdout at the moment, but doesn't throw
            morph.write(tmpDirectory / "bad-three-point-soma.swc");

            points.pop_back();
            CHECK_THROWS_AS(morph.write(tmpDirectory / "bad-three-point-soma.swc"),
                            morphio::WriterError);
        }
    }

    fs::remove_all(tmpDirectory);
}
