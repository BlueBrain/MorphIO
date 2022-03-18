#include <catch2/catch.hpp>

#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/section.h>

#include <algorithm>
#include <iterator>
#include <vector>

#include "test_helpers.h"


TEST_CASE("mitochondria", "[mitochondria]") {
    const auto mito = morphio::Morphology("data/h5/v1/mitochondria.h5").mitochondria();

    REQUIRE(mito.rootSections().size() == 2);

    auto rootSection = mito.rootSections().at(0);
    REQUIRE(rootSection.id() == 0);

    auto diameters = rootSection.diameters();
    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(diameters.begin(), diameters.end()),
                               std::vector<double>{10.0, 20.0},
                               0.01));
    auto relativePathLength = rootSection.relativePathLengths();
    auto res = std::vector<morphio::floatType>(relativePathLength.begin(),
                                               relativePathLength.end());

    REQUIRE(almost_equal(res.at(0), 0.5, 0.001));
    REQUIRE(almost_equal(res.at(1), 0.6000000238, 0.001));

    auto neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(neuriteSectionIds.begin(),
                                                               neuriteSectionIds.end()),
                               std::vector<double>{0.0, 0.0},
                               0.01));
    REQUIRE(rootSection.children().size() == 1);

    auto child = rootSection.children().at(0);
    REQUIRE(child.parent().id() == rootSection.id());

    diameters = child.diameters();
    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(diameters.begin(), diameters.end()),
                               std::vector<double>{20.0, 30.0, 40.0, 50.0},
                               0.01));
    relativePathLength = child.relativePathLengths();

    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(relativePathLength.begin(),
                                                               relativePathLength.end()),
                               std::vector<double>{0.6, 0.7, 0.8, 0.9},
                               0.01));

    neuriteSectionIds = child.neuriteSectionIds();
    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(neuriteSectionIds.begin(),
                                                               neuriteSectionIds.end()),
                               std::vector<double>{3.0, 4.0, 4.0, 5.0},
                               0.01));
    rootSection = mito.rootSections().at(1);
    diameters = rootSection.diameters();
    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(diameters.begin(), diameters.end()),
                               std::vector<double>{5.0, 6.0, 7.0, 8.0},
                               0.01));
    relativePathLength = rootSection.relativePathLengths();

    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(relativePathLength.begin(),
                                                               relativePathLength.end()),
                               std::vector<double>{0.6, 0.7, 0.8, 0.9},
                               0.01));

    neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE(array_almost_equal(std::vector<morphio::floatType>(neuriteSectionIds.begin(),
                                                               neuriteSectionIds.end()),
                               std::vector<double>{0.0, 1.0, 1.0, 2.0},
                               0.01));
    REQUIRE(rootSection.children().empty());
}

TEST_CASE("mitochondria.sections", "[mitochondria]") {
    const auto mito = morphio::Morphology("data/h5/v1/mitochondria.h5").mitochondria();
    auto sections = mito.sections();

    std::vector<std::size_t> res;
    std::transform(sections.begin(),
                   sections.end(),
                   std::back_inserter(res),
                   [](const morphio::MitoSection& s) { return s.id(); });
    REQUIRE(res == std::vector<size_t>{0, 1, 2});
}

TEST_CASE("mitochondria.iteration", "[mitochondria]") {
    const auto mito = morphio::Morphology("data/h5/v1/mitochondria.h5").mitochondria();

    const auto rootSection = mito.rootSections().at(0);

    std::vector<std::size_t> res;
    std::transform(rootSection.depth_begin(),
                   rootSection.depth_end(),
                   std::back_inserter(res),
                   [](const morphio::MitoSection& s) { return s.id(); });
    REQUIRE(res == std::vector<size_t>{0, 1});

    res.clear();
    std::transform(rootSection.breadth_begin(),
                   rootSection.breadth_end(),
                   std::back_inserter(res),
                   [](const morphio::MitoSection& s) { return s.id(); });
    REQUIRE(res == std::vector<size_t>{0, 1});

    res.clear();
    std::transform(rootSection.upstream_begin(),
                   rootSection.upstream_end(),
                   std::back_inserter(res),
                   [](const morphio::MitoSection& s) { return s.id(); });
    REQUIRE(res == std::vector<size_t>{0});
}

TEST_CASE("mitochondria.hasSameShape", "[mitochondria]") {
    morphio::Morphology morph0 = morphio::Morphology("data/h5/v1/mitochondria.h5");
    morphio::Morphology morph1 = morphio::Morphology("data/h5/v1/mitochondria.h5");
    REQUIRE(morph0.rootSections().at(0).hasSameShape(morph1.rootSections().at(0)));
}
