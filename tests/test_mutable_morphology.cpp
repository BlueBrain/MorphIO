#include "contrib/catch.hpp"

#include <morphio/mut/morphology.h>


TEST_CASE("RemoveRootsection", "[mutMorphology]") {
    // this test verifies we can delete a root section with recursive at false from a morphology
    // correctly. This is a special case where the root section as a single point and 2 child
    // sections. The number of sections is small enough to trigger a resize on _rootSections.
    morphio::mut::Morphology morpho("data/single_point_root.asc");
    for (const auto& rootSection : morpho.rootSections()) {
        if (rootSection->points().size() == 1) {
            morpho.deleteSection(rootSection, false);
        }
    }
    REQUIRE(morpho.rootSections().size() == 2);
}


TEST_CASE("RemoveUnifurcation", "[mutMorphology]") {
    morphio::mut::Morphology morpho("data/nested_single_children.asc");
    REQUIRE(morpho.rootSections().size() == 1);
    REQUIRE(morpho.rootSections()[0]->points().size() == 2);
    morpho.removeUnifurcations();
    REQUIRE(morpho.rootSections()[0]->points().size() == 5);
}
