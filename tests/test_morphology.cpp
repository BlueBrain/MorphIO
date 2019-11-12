#include "contrib/catch.hpp"

#include <morphio/morphology.h>


TEST_CASE("LoadH5Morphology", "[morphology]") {
    const morphio::Morphology m("data/h5/v1/Neuron.h5");

    REQUIRE(m.diameters().size() == 924);
}
