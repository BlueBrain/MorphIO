#include "contrib/catch.hpp"
#include <cmath>
#include <gsl/gsl>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/glial_cell.h>
#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/soma.h>

float roundf(float a) {
    return std::ceil(a * 100.0f) / 100.0f;
}

double round(double a) {
    return std::ceil(a * 100.0) / 100.0;
}

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

TEST_CASE("fromMut", "[immutableMorphology]") {
    Files files;
    std::vector<morphio::Morphology> morphs;
    for (const auto& names : files.fileNames) {
        morphio::mut::Morphology mutMorph = morphio::mut::Morphology(names);
        morphs.emplace_back(morphio::Morphology(mutMorph));
    }
    const auto& expectedMorphs = files.morphs();
    for (size_t i = 0; i < expectedMorphs.size(); ++i) {
        REQUIRE(expectedMorphs.at(i).somaType() == morphs.at(i).somaType());
        REQUIRE(expectedMorphs.at(i).sectionTypes() == morphs.at(i).sectionTypes());
        REQUIRE(expectedMorphs.at(i).perimeters() == morphs.at(i).perimeters());
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


TEST_CASE("distance", "[immutableMorphology]") {
    Files files;
    for (const auto& morph : files.morphs()) {
        REQUIRE(morph.soma().maxDistance() == 0);
    }
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
    uint32_t major;
    uint32_t minor;
    std::tie(text, major, minor) = files.morphs().at(0).version();
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
            std::vector<morphio::floatType>{morphio::floatType(10.0), morphio::floatType(20.0)});
    auto relativePathLength = rootSection.relativePathLengths();
    auto res = std::vector<morphio::floatType>(relativePathLength.begin(),
                                               relativePathLength.end());


#ifdef MORPHIO_USE_DOUBLE
    REQUIRE(round(res.at(0)) == round(morphio::floatType(0.5)));
    REQUIRE(round(res.at(1)) == round(morphio::floatType(0.6000000238)));
#else
    REQUIRE(roundf(res.at(0)) == roundf(morphio::floatType(0.5)));
    REQUIRE(roundf(res.at(1)) == roundf(morphio::floatType(0.6000000238)));
#endif

    auto neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE(std::vector<morphio::floatType>(neuriteSectionIds.begin(), neuriteSectionIds.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(0.0), morphio::floatType(0.0)});
    REQUIRE(rootSection.children().size() == 1);

    auto child = rootSection.children().at(0);
    REQUIRE(child.parent().id() == rootSection.id());

    diameters = child.diameters();
    REQUIRE(std::vector<morphio::floatType>(diameters.begin(), diameters.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(20.0),
                                            morphio::floatType(30.0),
                                            morphio::floatType(40.0),
                                            morphio::floatType(50.0)});
    relativePathLength = child.relativePathLengths();
    REQUIRE(std::vector<morphio::floatType>(relativePathLength.begin(), relativePathLength.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(0.6),
                                            morphio::floatType(0.7),
                                            morphio::floatType(0.8),
                                            morphio::floatType(0.9)});
    neuriteSectionIds = child.neuriteSectionIds();
    REQUIRE(std::vector<morphio::floatType>(neuriteSectionIds.begin(), neuriteSectionIds.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(3.0),
                                            morphio::floatType(4.0),
                                            morphio::floatType(4.0),
                                            morphio::floatType(5.0)});

    rootSection = mito.rootSections().at(1);

    diameters = rootSection.diameters();
    REQUIRE(std::vector<morphio::floatType>(diameters.begin(), diameters.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(5.0),
                                            morphio::floatType(6.0),
                                            morphio::floatType(7.0),
                                            morphio::floatType(8.0)});
    relativePathLength = rootSection.relativePathLengths();
    REQUIRE(std::vector<morphio::floatType>(relativePathLength.begin(), relativePathLength.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(0.6),
                                            morphio::floatType(0.7),
                                            morphio::floatType(0.8),
                                            morphio::floatType(0.9)});
    neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE(std::vector<morphio::floatType>(neuriteSectionIds.begin(), neuriteSectionIds.end()) ==
            std::vector<morphio::floatType>{morphio::floatType(0.0),
                                            morphio::floatType(1.0),
                                            morphio::floatType(1.0),
                                            morphio::floatType(2.0)});
    REQUIRE(rootSection.children().size() == 0);
}


TEST_CASE("endoplasmic_reticulum", "[immutableMorphology]") {
    morphio::Morphology morph = morphio::Morphology("data/h5/v1/endoplasmic-reticulum.h5");
    morphio::EndoplasmicReticulum er = morph.endoplasmicReticulum();
    REQUIRE(er.sectionIndices() == std::vector<uint32_t>{1, 4, 5});
#ifdef MORPHIO_USE_DOUBLE
    REQUIRE(round(er.volumes().at(0)) == round(morphio::floatType(10.5500001907)));
    REQUIRE(round(er.volumes().at(1)) == round(morphio::floatType(47.1199989319)));
    REQUIRE(round(er.volumes().at(2)) == round(morphio::floatType(0.8299999833)));
#else
    REQUIRE(roundf(er.volumes().at(0)) == roundf(morphio::floatType(10.5500001907)));
    REQUIRE(roundf(er.volumes().at(1)) == roundf(morphio::floatType(47.1199989319)));
    REQUIRE(roundf(er.volumes().at(2)) == roundf(morphio::floatType(0.8299999833)));
#endif

    REQUIRE(er.surfaceAreas() == std::vector<morphio::floatType>{morphio::floatType(111.24),
                                                                 morphio::floatType(87.44),
                                                                 morphio::floatType(0.11)});
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

TEST_CASE("markers", "[immutableMorphology]") {
    morphio::Morphology morph = morphio::Morphology("data/pia.asc");
    std::vector<morphio::Property::Marker> markers = morph.markers();
    REQUIRE(markers.at(0)._label == "pia");
}

TEST_CASE("throws", "[immutableMorphology]") {
    bool throwCorrectError = false;
    try {
        auto fail = morphio::Morphology("data");
        FAIL();
    } catch (morphio::UnknownFileType&) {
        throwCorrectError = true;
    }
    REQUIRE(throwCorrectError);

    try {
        auto fail = morphio::Morphology("data/unknown.asc");
        FAIL();
    } catch (morphio::RawDataError&) {
        throwCorrectError = true;
    }
    REQUIRE(throwCorrectError);

    try {
        auto fail = morphio::Morphology("data/simple.unknown");
        FAIL();
    } catch (morphio::UnknownFileType&) {
        throwCorrectError = true;
    }
    REQUIRE(throwCorrectError);
}

TEST_CASE("annotations", "[immutableMorphology]") {
    auto mutMorph = morphio::mut::Morphology("data/annotations.asc");
    mutMorph.removeUnifurcations();
    REQUIRE(mutMorph.annotations().size() == 1);
    auto morph = morphio::Morphology(mutMorph);
    REQUIRE(morph.annotations().size() == 1);
    auto annotation = morph.annotations().at(0);
    REQUIRE(annotation._sectionId == 1);
    REQUIRE(annotation._type == morphio::SINGLE_CHILD);
}
