/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <catch2/catch.hpp>

#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/section.h>

#include <algorithm>
#include <iterator>
#include <vector>

namespace {
using floatTypes = std::vector<morphio::floatType>;
}  // namespace

TEST_CASE("mitochondria", "[mitochondria]") {
    morphio::Morphology morph = morphio::Morphology("data/h5/v1/mitochondria.h5");
    morphio::Mitochondria mito = morph.mitochondria();
    REQUIRE(mito.rootSections().size() == 2);

    morphio::MitoSection rootSection = mito.rootSections().at(0);
    REQUIRE(rootSection.id() == 0);
    auto diameters = rootSection.diameters();
    REQUIRE_THAT(floatTypes(diameters.begin(), diameters.end()),
                 Catch::Approx(floatTypes{10.0, 20.0}));
    auto relativePathLength = rootSection.relativePathLengths();

    auto res = floatTypes(relativePathLength.begin(), relativePathLength.end());
    REQUIRE_THAT(res.at(0), Catch::WithinAbs(0.5, 0.0001));
    REQUIRE_THAT(res.at(1), Catch::WithinAbs(0.6, 0.0001));

    auto neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE_THAT(floatTypes(neuriteSectionIds.begin(), neuriteSectionIds.end()),
                 Catch::Approx(floatTypes{0.0, 0.0}));
    REQUIRE(rootSection.children().size() == 1);

    auto child = rootSection.children()[0];
    REQUIRE(child.parent().id() == rootSection.id());

    diameters = child.diameters();
    REQUIRE_THAT(floatTypes(diameters.begin(), diameters.end()),
                 Catch::Approx(floatTypes{20.0, 30.0, 40.0, 50.0}));
    relativePathLength = child.relativePathLengths();

    REQUIRE_THAT(floatTypes(relativePathLength.begin(), relativePathLength.end()),
                 Catch::Approx(floatTypes{0.6, 0.7, 0.8, 0.9}));

    neuriteSectionIds = child.neuriteSectionIds();
    REQUIRE_THAT(floatTypes(neuriteSectionIds.begin(), neuriteSectionIds.end()),
                 Catch::Approx(floatTypes{3.0, 4.0, 4.0, 5.0}));

    rootSection = mito.rootSections().at(1);
    diameters = rootSection.diameters();
    REQUIRE_THAT(floatTypes(diameters.begin(), diameters.end()),
                 Catch::Approx(floatTypes{5.0, 6.0, 7.0, 8.0}));
    relativePathLength = rootSection.relativePathLengths();

    REQUIRE_THAT(floatTypes(relativePathLength.begin(), relativePathLength.end()),
                 Catch::Approx(floatTypes{0.6, 0.7, 0.8, 0.9}));

    neuriteSectionIds = rootSection.neuriteSectionIds();
    REQUIRE_THAT(floatTypes(neuriteSectionIds.begin(), neuriteSectionIds.end()),
                 Catch::Approx(floatTypes{0.0, 1.0, 1.0, 2.0}));
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

    const auto rootSection = mito.rootSections()[0];

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

TEST_CASE("mitochondria.hasSameShape") {
    const auto morph0 = morphio::Morphology("data/h5/v1/mitochondria.h5");
    const auto morph1 = morphio::Morphology("data/h5/v1/mitochondria.h5");

    morphio::Mitochondria mito0 = morph0.mitochondria();
    morphio::Mitochondria mito1 = morph1.mitochondria();

    CHECK(mito0.rootSections()[0].hasSameShape(mito1.rootSections()[0]) == true);
}
