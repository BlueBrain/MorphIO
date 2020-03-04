#include "contrib/catch.hpp"

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
