#include <locale>

#include "../src/readers/morphologyHDF5.h"
#include <catch2/catch.hpp>

#include <highfive/H5File.hpp>
#include <morphio/dendritic_spine.h>
#include <morphio/enums.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>
#include <morphio/soma.h>

TEST_CASE("LoadH5Morphology", "[morphology]") {
    {
        const morphio::Morphology m("data/h5/v1/Neuron.h5");
        REQUIRE(m.soma().points().size() == 3);
        REQUIRE(m.diameters().size() == 924);
        REQUIRE(m.points().size() == 924);
        // 3 point soma
        REQUIRE(m.somaType() == morphio::SomaType::SOMA_SIMPLE_CONTOUR);
    }

    {
        const morphio::Morphology m("data/h5/v1/Neuron-no-soma.h5");
        REQUIRE(m.diameters().size() == 924);
        REQUIRE(m.points().size() == 924);
        REQUIRE(m.somaType() == morphio::SomaType::SOMA_UNDEFINED);
    }

    {
        const morphio::Morphology m("data/h5/v1/simple.h5");
        REQUIRE(m.soma().points().size() == 1);
        REQUIRE(m.diameters().size() == 12);
        REQUIRE(m.points().size() == 12);
        // 1 point soma
        REQUIRE(m.somaType() == morphio::SomaType::SOMA_SINGLE_POINT);
    }

    {
        const morphio::Morphology m("data/h5/v1/simple-two-point-soma.h5");
        REQUIRE(m.soma().points().size() == 2);
        REQUIRE(m.diameters().size() == 12);
        REQUIRE(m.points().size() == 12);
        // 2 point soma
        REQUIRE(m.somaType() == morphio::SomaType::SOMA_UNDEFINED);
    }

    {
        // This is to cover the appendProperties perimeters line in mut/morphology.cpp,
        // which is triggered if modifiers are used in a morphology that has perimeters
        const morphio::Morphology m("data/h5/v1/glia.h5", morphio::enums::Option::NRN_ORDER);
        REQUIRE(m.soma().points().size() == 2);
        REQUIRE(m.points().size() == 2);
        REQUIRE(m.perimeters().size() == 2);
    }

    {  // file is an not a valid h5 file
        CHECK_THROWS_AS(morphio::Morphology("data/h5/non-valid.h5"), morphio::RawDataError);
    }

    {  // h5v2 is not supported
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v2/Neuron.h5"), morphio::RawDataError);
    }

    {  // empty h5 file doesn't have /points or /structure
        CHECK_THROWS_AS(morphio::Morphology("data/h5/empty.h5"), morphio::RawDataError);
    }

    {  // empty metadata group
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/metadata_group_only.h5"),
                        morphio::RawDataError);
    }

    {  // unsupported version number
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/h5v1.4.h5"), morphio::RawDataError);
    }

    {  // incorrect points shape
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/incorrect_point_columns.h5"),
                        morphio::RawDataError);
    }

    {  // incorrect structure shape
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/incorrect_structure_columns.h5"),
                        morphio::RawDataError);
    }

    {  // incorrect soma section structure
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/three-point-soma-two-offset.h5"),
                        morphio::RawDataError);
    }

    {  // incorrect type in /structure
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/unknown_section_type_structure.h5"),
                        morphio::RawDataError);
    }

    {  // soma after dendrite
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/soma_after_dendrite.h5"),
                        morphio::RawDataError);
    }
}

TEST_CASE("LoadH5Glia", "[morphology]") {
    {
        const morphio::Morphology m("data/h5/v1/glia.h5");
        REQUIRE(m.soma().points().size() == 2);
        REQUIRE(m.points().size() == 2);
        REQUIRE(m.perimeters().size() == 2);
    }
    {
        const morphio::Morphology m("data/h5/v1/glia_soma_only.h5");
        REQUIRE(m.soma().points().size() == 1);
        REQUIRE(m.points().empty());
        REQUIRE(m.perimeters().empty());
    }

    {  // empty perimeters
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/glia_empty_perimeters.h5"),
                        morphio::RawDataError);
    }

    {  // wrong sized perimeters
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/glia_wrong_sized_perimeters.h5"),
                        morphio::RawDataError);
    }
}

TEST_CASE("LoadH5DendriticSpine", "[DendriticSpine]") {
    {
        const morphio::DendriticSpine d("data/h5/v1/simple-dendritric-spine.h5");
        REQUIRE(d.points().size() == 8);
        REQUIRE(d.postSynapticDensity().size() == 2);
    }
    {
        const morphio::Morphology m("data/h5/v1/glia_soma_only.h5");
        REQUIRE(m.soma().points().size() == 1);
        REQUIRE(m.points().empty());
        REQUIRE(m.perimeters().empty());
    }

    {  // empty perimeters
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/glia_empty_perimeters.h5"),
                        morphio::RawDataError);
    }

    {  // wrong sized perimeters
        CHECK_THROWS_AS(morphio::Morphology("data/h5/v1/glia_wrong_sized_perimeters.h5"),
                        morphio::RawDataError);
    }
}

TEST_CASE("LoadH5MorphologySingleNeurite", "[morphology]") {
    {
        const morphio::Morphology m("data/h5/v1/single-neurite.h5");
        REQUIRE(m.soma().points().empty());
        REQUIRE(m.points().size() == 3);
        REQUIRE_THAT(m.points()[0][0], Catch::WithinAbs(4., 0.0001));
    }
}

TEST_CASE("LoadSWCMorphology", "[morphology]") {
    const morphio::Morphology m("data/simple.swc");

    REQUIRE(m.diameters().size() == 12);
}

class LocaleGuard {
public:
    explicit LocaleGuard(const std::locale& newLocale)
        : previousLocale(std::locale())
    {
        previousLocale = std::locale::global(newLocale);
    }

    ~LocaleGuard() {
        std::locale::global(previousLocale);
    }

private:
    std::locale previousLocale;
};

TEST_CASE("LoadSWCMorphologyLocale", "[morphology]") {
    {
        const auto locale = LocaleGuard(std::locale("de_DE.UTF-8"));
        const morphio::Morphology m("data/simple.swc");
        REQUIRE(m.diameters().size() == 12);
    }
}

TEST_CASE("LoadNeurolucidaMorphology", "[morphology]") {
    const morphio::Morphology m("data/multiple_point_section.asc");

    REQUIRE(m.diameters().size() == 14);
}

TEST_CASE("LoadNeurolucidaMorphologyMarkers", "[morphology]") {
    const morphio::Morphology m("data/markers.asc");

    REQUIRE(m.markers().size() == 5);

    {  // file is an not a valid h5 file
        CHECK_THROWS_AS(morphio::Morphology("data/invalid-incomplete.asc"), morphio::RawDataError);
    }
}

TEST_CASE("LoadBadDimensionMorphology", "[morphology]") {
    REQUIRE_THROWS(morphio::Morphology("data/h5/v1/monodim.h5"));
}

TEST_CASE("LoadMergedMorphology", "[morphology]") {
    auto file = HighFive::File("data/h5/merged.h5", HighFive::File::ReadOnly);
    REQUIRE_NOTHROW(morphio::readers::h5::MorphologyHDF5(
        file.getGroup("/00/00/00000009b4fa102d58b173a995525c3e")));
    auto g = file.getGroup("/00/00/00000009b4fa102d58b173a995525c3e");
    morphio::Morphology m(g);
    REQUIRE(m.rootSections().size() == 8);
}
