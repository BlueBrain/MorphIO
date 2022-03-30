#include <cmath>
#include <limits>
#include <sstream>

#include <catch2/catch.hpp>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/glial_cell.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vector_types.h>

namespace {

class Files
{
  public:
    Files()
        : fileNames({"data/simple.asc", "data/simple.swc", "data/h5/v1/simple.h5"}) {}
    std::vector<std::string> fileNames;
    std::vector<morphio::Morphology> morphs() {
        std::vector<morphio::Morphology> res;
        for (const auto& name : fileNames) {
            res.emplace_back(name);
        }
        return res;
    }
};
}  // anonymous namespace

TEST_CASE("fromMut", "[immutableMorphology]") {
    Files files;
    std::vector<morphio::Morphology> morphs;
    for (const auto& names : files.fileNames) {
        morphio::mut::Morphology mutMorph = morphio::mut::Morphology(names);
        morphs.emplace_back(morphio::Morphology(mutMorph));
    }
    const auto& expectedMorphs = files.morphs();
    for (size_t i = 0; i < expectedMorphs.size(); ++i) {
        REQUIRE(expectedMorphs[i].somaType() == morphs[i].somaType());
        REQUIRE(expectedMorphs[i].sectionTypes() == morphs[i].sectionTypes());
        REQUIRE(expectedMorphs[i].perimeters() == morphs[i].perimeters());
    }
}

TEST_CASE("sections", "[immutableMorphology]") {
    Files files;
    for (const auto& morph : files.morphs()) {
        for (const morphio::Section& section : morph.rootSections()) {
            REQUIRE(section.isRoot());
            for (const morphio::Section& child : section.children()) {
                REQUIRE(!child.isRoot());
            }
        }
        auto sections = morph.sections();
        REQUIRE(sections.size() == 6);
    }
    {
        morphio::Morphology morph0 = morphio::Morphology("data/h5/v1/simple.h5");
        morphio::Morphology morph1 = morphio::Morphology("data/h5/v1/simple.h5");
        REQUIRE(morph0.rootSections()[0].hasSameShape(morph1.rootSections()[0]));
        REQUIRE(!morph0.rootSections()[0].hasSameShape(morph1.rootSections()[1]));
    }
}

TEST_CASE("heterogeneous-sections", "[immutableMorphology]") {
    auto morph = morphio::Morphology("data/simple-heterogeneous-neurite.swc");

    /** The morphology consists of two trees, with one bifurcation each. The root
     * sections had a different type than their respective children.
     */
    for (const auto& root_section : morph.rootSections()) {
        // We expect the root section to be heterogeneous downstream because
        // of their children of different type and homogeneous upstream because
        // there are no other sections.
        REQUIRE(root_section.isHeterogeneous(true));       // downstream = true
        REQUIRE(not root_section.isHeterogeneous(false));  // downstream = false

        // We expect the two children for each root section, which have a different
        // type to be homogeneous downstream as leaves and inhomogeneous upstream because
        // of the root section parent of different type.
        for (const auto& section : root_section.children()) {
            REQUIRE(not section.isHeterogeneous(true));  // downstream = true
            REQUIRE(section.isHeterogeneous(false));     // downstream = false
        }
    }
}

TEST_CASE("modifers", "[immutableMorphology]") {
    morphio::Morphology morphNoModifier = morphio::Morphology(
        "data/reversed_NRN_neurite_order.swc");
    std::vector<morphio::SectionType> rootSectionTypesNoModifier;
    for (auto sectionNoMod : morphNoModifier.rootSections()) {
        rootSectionTypesNoModifier.push_back(sectionNoMod.type());
    }
    REQUIRE(rootSectionTypesNoModifier == std::vector<morphio::SectionType>{
                                              morphio::SECTION_APICAL_DENDRITE,
                                              morphio::SECTION_DENDRITE,
                                              morphio::SECTION_AXON,
                                          });

    morphio::Morphology morph = morphio::Morphology("data/reversed_NRN_neurite_order.swc",
                                                    morphio::Option::NRN_ORDER);

    std::vector<morphio::SectionType> rootSectionTypes;
    for (auto section : morph.rootSections()) {
        rootSectionTypes.push_back(section.type());
    }
    REQUIRE(rootSectionTypes.size() == 3);
    REQUIRE(rootSectionTypes ==
            std::vector<morphio::SectionType>{morphio::SECTION_AXON,
                                              morphio::SECTION_DENDRITE,
                                              morphio::SECTION_APICAL_DENDRITE});


    morphio::Morphology morphModifierh5 = morphio::Morphology("data/h5/v1/simple.h5",
                                                              morphio::Option::NRN_ORDER);

    std::vector<morphio::SectionType> rootSectionTypesH5;
    for (auto section : morphModifierh5.rootSections()) {
        rootSectionTypesH5.push_back(section.type());
    }
    // Should be inverted without the option
    REQUIRE(rootSectionTypesH5 == std::vector<morphio::SectionType>{
                                      morphio::SECTION_AXON,
                                      morphio::SECTION_DENDRITE,
                                  });
}


TEST_CASE("immutableMorphologySoma", "[immutableMorphology]") {
    Files files;
    for (const auto& morph : files.morphs()) {
        REQUIRE(morph.soma().maxDistance() == 0);
    }

    const auto morph = morphio::Morphology("data/soma_three_points_cylinder.swc");
    const auto& soma = morph.soma();

    REQUIRE_THAT(soma.center()[0], Catch::WithinAbs(0, 0.001));
    REQUIRE_THAT(soma.center()[1], Catch::WithinAbs(0, 0.001));
    REQUIRE_THAT(soma.center()[2], Catch::WithinAbs(0, 0.001));

    REQUIRE_THAT(soma.surface(), Catch::WithinAbs(1017.87604, 0.001));
}

TEST_CASE("properties", "[immutableMorphology]") {
    Files files;
    for (const auto& morph : files.morphs()) {
        REQUIRE(morph.somaType() == morphio::enums::SomaType::SOMA_SINGLE_POINT);
        auto perimeters = morph.perimeters();
        REQUIRE(std::vector<morphio::floatType>(perimeters.begin(), perimeters.end())
                    .empty());  // empty
                                // atm
        const auto& sectionTypes = morph.sectionTypes();
        REQUIRE(std::vector<morphio::SectionType>(sectionTypes.begin(), sectionTypes.end()) ==
                std::vector<morphio::SectionType>{morphio::SECTION_DENDRITE,
                                                  morphio::SECTION_DENDRITE,
                                                  morphio::SECTION_DENDRITE,
                                                  morphio::SECTION_AXON,
                                                  morphio::SECTION_AXON,
                                                  morphio::SECTION_AXON});
    }
    std::string text;
    uint32_t major = std::numeric_limits<uint32_t>::max();
    uint32_t minor = std::numeric_limits<uint32_t>::max();
    std::tie(text, major, minor) = files.morphs()[0].version();
    REQUIRE(text == "asc");
    REQUIRE(major == 1);
    REQUIRE(minor == 0);
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
        REQUIRE((*iter).id() == expectedRootSectionId[count++]);
    }
    std::vector<size_t> expectedMorphSectionId = {0, 7, 1, 4, 8, 9, 2, 3, 5, 6};
    count = 0;
    for (auto iter = iterMorph.breadth_begin(); iter != iterMorph.breadth_end(); iter++) {
        REQUIRE((*iter).id() == expectedMorphSectionId[count++]);
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
                REQUIRE(point == expectedPoints[count++]);
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

TEST_CASE("endoplasmic_reticulum", "[immutableMorphology]") {
    morphio::Morphology morph = morphio::Morphology("data/h5/v1/endoplasmic-reticulum.h5");
    morphio::EndoplasmicReticulum er = morph.endoplasmicReticulum();
    REQUIRE(er.sectionIndices() == std::vector<uint32_t>{1, 4, 5});
    REQUIRE_THAT(er.volumes().at(0), Catch::WithinAbs(10.5500001907, 0.01));
    REQUIRE_THAT(er.volumes().at(1), Catch::WithinAbs(47.1199989319, 0.01));
    REQUIRE_THAT(er.volumes().at(2), Catch::WithinAbs(0.8299999833, 0.01));
    REQUIRE_THAT(er.surfaceAreas(), Catch::Approx(std::vector<morphio::floatType>{111.24, 87.44, 0.11}));
    REQUIRE(er.filamentCounts() == std::vector<uint32_t>{12, 42, 8});
}

TEST_CASE("glia", "[immutableMorphology]") {
    morphio::GlialCell glial = morphio::GlialCell("data/astrocyte.h5");
    REQUIRE(glial.cellFamily() == morphio::CellFamily::GLIA);

    auto section_types = glial.sectionTypes();

    size_t count_processes = 0;
    size_t count_perivascular_processes = 0;

    for (const auto type : section_types) {
        if (type == morphio::SECTION_GLIA_PERIVASCULAR_PROCESS)
            ++count_perivascular_processes;
        else if (type == morphio::SECTION_GLIA_PROCESS)
            ++count_processes;
        else
            REQUIRE(false);
    }

    REQUIRE(count_perivascular_processes == 452);
    REQUIRE(count_processes == 863);

    const auto section = glial.rootSections()[0];
    REQUIRE_THAT(section.diameters()[0], Catch::WithinAbs(2.03101, 0.001));
    REQUIRE_THAT(section.diameters()[1], Catch::WithinAbs(1.86179, 0.001));

    REQUIRE_THAT(section.perimeters()[0], Catch::WithinAbs(5.79899, 0.001));
    REQUIRE_THAT(section.perimeters()[1], Catch::WithinAbs(7.98946, 0.001));

    CHECK_THROWS_AS(morphio::GlialCell("data/simple.swc"), morphio::RawDataError);
    CHECK_THROWS_AS(morphio::GlialCell("data/h5/v1/simple.h5"), morphio::RawDataError);
}

TEST_CASE("markers", "[immutableMorphology]") {
    morphio::Morphology morph = morphio::Morphology("data/pia.asc");
    std::vector<morphio::Property::Marker> markers = morph.markers();
    REQUIRE(markers[0]._label == "pia");
}

TEST_CASE("throws", "[immutableMorphology]") {
    CHECK_THROWS_AS(morphio::Morphology("data"), morphio::UnknownFileType);
    CHECK_THROWS_AS(morphio::Morphology("data/unknown.asc"), morphio::RawDataError);
    CHECK_THROWS_AS(morphio::Morphology("data/simple.unknown"), morphio::UnknownFileType);
}

TEST_CASE("annotations", "[immutableMorphology]") {
    auto mutMorph = morphio::mut::Morphology("data/annotations.asc");
    mutMorph.removeUnifurcations();
    REQUIRE(mutMorph.annotations().size() == 1);

    auto morph = morphio::Morphology(mutMorph);
    REQUIRE(morph.annotations().size() == 1);

    auto annotation = morph.annotations()[0];
    REQUIRE(annotation._sectionId == 1);
    REQUIRE(annotation._type == morphio::SINGLE_CHILD);
}

TEST_CASE("operator<<", "[immutableMorphology]") {
    // TODO: make this more comprehensive
    morphio::GlialCell glial = morphio::GlialCell("data/astrocyte.h5");
    const auto section = glial.rootSections()[0];

    std::stringstream ss;

    ss << section;
}
