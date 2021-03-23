#include "contrib/catch.hpp"
#include <gsl/gsl>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/glial_cell.h>
#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/soma.h>


class Files
{
  public:
    Files()
        : fileNames({"data/simple.asc", "data/simple.swc", "data/h5/v1/simple.h5"}){};
    std::vector<std::string> fileNames;
    std::vector<morphio::Morphology> morphs() {
        std::vector<morphio::Morphology> res;
        for (const auto& name : fileNames) {
            res.emplace_back(name);
        }
        return res;
    }
};


TEST_CASE("isRoot", "[immutableMorphology]") {
    Files files;
    for (const auto& morph : files.morphs()) {
        for (const morphio::Section& section : morph.rootSections()) {
            REQUIRE(section.isRoot());
            for (const morphio::Section& child : section.children()) {
                REQUIRE(!child.isRoot());
            }
        }
    }
}

TEST_CASE("distance", "[immutableMorphology]") {
    Files files;
    for (const auto& morph : files.morphs()) {
        REQUIRE(morph.soma().maxDistance() == 0);
    }
}

TEST_CASE("iter", "[immutableMorphology]") {
    morphio::Morphology iterMorph = morphio::Morphology("data/iterators.asc");
    auto rootSection = iterMorph.rootSections()[0];
    size_t count = 0;
    for (auto iter = rootSection.depth_begin(); iter != rootSection.depth_end(); iter++) {
        REQUIRE((*iter).id() == count++);
    }
    std::vector<size_t> expectedRootSectionId = {0, 1, 4, 2, 3, 5, 6};
    count = 0;
    for (auto iter = rootSection.breadth_begin(); iter != rootSection.breadth_end(); iter++) {
        REQUIRE((*iter).id() == expectedRootSectionId.at(count++));
    }
    std::vector<size_t> expectedMorphSectionId = {0, 7, 1, 4, 8, 9, 2, 3, 5, 6};
    count = 0;
    for (auto iter = iterMorph.breadth_begin(); iter != iterMorph.breadth_end(); iter++) {
        REQUIRE((*iter).id() == expectedMorphSectionId.at(count++));
    }

    Files files;
    for (const auto& morph : files.morphs()) {
        count = 0;
        for (auto iter = morph.depth_begin(); iter != morph.depth_end(); iter++) {
            REQUIRE((*iter).id() == count++);
        }
        auto testedSection = morph.rootSections()[0].children()[0];
        std::vector<morphio::Point> expectedPoints = {{0., 5., 0.},
                                                      {-5., 5., 0.},
                                                      {0., 0., 0.},
                                                      {0., 5., 0.}};
        count = 0;
        for (auto iter = testedSection.upstream_begin(); iter != testedSection.upstream_end();
             iter++) {
            auto points = (*iter).points();
            for (auto point : points) {
                REQUIRE(point == expectedPoints.at(count++));
            }
        }
    }
}

TEST_CASE("section_offsets", "[immutableMorphology]") {
    Files files;
    std::vector<uint32_t> expectedSectionOffsets = {0, 2, 4, 6, 8, 10, 12};
    for (const auto& morph : files.morphs()) {
        REQUIRE(morph.sectionOffsets() == expectedSectionOffsets);
    }
}

TEST_CASE("connectivity", "[immutableMorphology]") {
    Files files;
    std::map<int, std::vector<unsigned int>> expectedConnectivity = {{-1, {0, 3}},
                                                                     {0, {1, 2}},
                                                                     {3, {4, 5}}};
    for (const auto& morph : files.morphs()) {
        REQUIRE(morph.connectivity() == expectedConnectivity);
    }
}

TEST_CASE("mitochondria", "[immutableMorphology]") {
    morphio::Morphology morph = morphio::Morphology("data/h5/v1/mitochondria.h5");
    morphio::Mitochondria mito = morph.mitochondria();
    REQUIRE(mito.rootSections().size() == 2);
    morphio::MitoSection rootSection = mito.rootSections().at(0);
    REQUIRE(rootSection.id() == 0);
    auto diameters = rootSection.diameters();
    REQUIRE(std::vector<morphio::floatType>(diameters.begin(), diameters.end()) ==
            std::vector<morphio::floatType>{10.f, 20.f});
    auto relativePathLength = rootSection.relativePathLengths();
    REQUIRE(std::vector<morphio::floatType>(relativePathLength.begin(), relativePathLength.end()) ==
            std::vector<morphio::floatType>{0.5f, 0.6f});
    auto neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE(std::vector<morphio::floatType>(neuriteSectionIds.begin(), neuriteSectionIds.end()) ==
            std::vector<morphio::floatType>{0.f, 0.f});
    REQUIRE(rootSection.children().size() == 1);

    auto child = rootSection.children().at(0);
    REQUIRE(child.parent().id() == rootSection.id());

    diameters = child.diameters();
    REQUIRE(std::vector<morphio::floatType>(diameters.begin(), diameters.end()) ==
            std::vector<morphio::floatType>{20.f, 30.f, 40.f, 50.f});
    relativePathLength = child.relativePathLengths();
    REQUIRE(std::vector<morphio::floatType>(relativePathLength.begin(), relativePathLength.end()) ==
            std::vector<morphio::floatType>{0.6f, 0.7f, 0.8f, 0.9f});
    neuriteSectionIds = child.neuriteSectionIds();
    REQUIRE(std::vector<morphio::floatType>(neuriteSectionIds.begin(), neuriteSectionIds.end()) ==
            std::vector<morphio::floatType>{3.f, 4.f, 4.f, 5.f});

    rootSection = mito.rootSections().at(1);

    diameters = rootSection.diameters();
    REQUIRE(std::vector<morphio::floatType>(diameters.begin(), diameters.end()) ==
            std::vector<morphio::floatType>{5.0f, 6.0f, 7.0f, 8.0f});
    relativePathLength = rootSection.relativePathLengths();
    REQUIRE(std::vector<morphio::floatType>(relativePathLength.begin(), relativePathLength.end()) ==
            std::vector<morphio::floatType>{0.6f, 0.7f, 0.8f, 0.9f});
    neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE(std::vector<morphio::floatType>(neuriteSectionIds.begin(), neuriteSectionIds.end()) ==
            std::vector<morphio::floatType>{0.0f, 1.0f, 1.0f, 2.0f});
    REQUIRE(rootSection.children().size() == 0);
}

TEST_CASE("endoplasmic_reticulum", "[immutableMorphology]") {
    morphio::Morphology morph = morphio::Morphology("data/h5/v1/endoplasmic-reticulum.h5");
    morphio::EndoplasmicReticulum er = morph.endoplasmicReticulum();
    REQUIRE(er.sectionIndices() == std::vector<uint32_t>{1, 4, 5});
    REQUIRE(er.volumes() == std::vector<morphio::floatType>{10.55f, 47.12f, 0.83f});
    REQUIRE(er.surfaceAreas() == std::vector<morphio::floatType>{111.24f, 87.44f, 0.11f});
    REQUIRE(er.filamentCounts() == std::vector<uint32_t>{12, 42, 8});
}

TEST_CASE("glia", "[immutableMorphology]") {
    morphio::GlialCell glial = morphio::GlialCell("data/astrocyte.h5");
    REQUIRE(glial.cellFamily() == morphio::CellFamily::GLIA);
    bool throwCorrectError = false;
    try {
        auto fail = morphio::GlialCell("data/simple.swc");
        FAIL();
    } catch (morphio::RawDataError&) {
        throwCorrectError = true;
    }
    REQUIRE(throwCorrectError);
    throwCorrectError = false;
    try {
        auto fail = morphio::GlialCell("data/h5/v1/simple.h5");
        FAIL();
    } catch (morphio::RawDataError&) {
        throwCorrectError = true;
    }
    REQUIRE(throwCorrectError);
}
