#include "../src/readers/morphologyHDF5.h"
#include "contrib/catch.hpp"

#include <highfive/H5File.hpp>
#include <morphio/morphology.h>


TEST_CASE("LoadH5Morphology", "[morphology]") {
    const morphio::Morphology m("data/h5/v1/Neuron.h5");

    REQUIRE(m.diameters().size() == 924);
}

TEST_CASE("LoadSWCMorphology", "[morphology]") {
    const morphio::Morphology m("data/simple.swc");

    REQUIRE(m.diameters().size() == 12);
}

TEST_CASE("LoadNeurolucidaMorphology", "[morphology]") {
    const morphio::Morphology m("data/multiple_point_section.asc");

    REQUIRE(m.diameters().size() == 14);
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

TEST_CASE("LoadSinglePointRootTest", "[morphology]") {
    const morphio::Morphology m("data/single_point_root.asc");
    REQUIRE(m.rootSections()[0].isRoot() == true);
}


TEST_CASE("LoadSinglePointRoot", "[morphology]") {
    const morphio::Morphology m("data/single_point_root.asc", morphio::enums::Option::NO_SINGLE_POINT_ROOT);
//    const morphio::Morphology m("data/single_point_root.asc");
    REQUIRE(m.rootSections().size() == 2);
}
