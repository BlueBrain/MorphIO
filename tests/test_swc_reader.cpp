/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/exceptions.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <catch2/catch.hpp>

using namespace morphio;


TEST_CASE("morphio::swc::errors") {
    SECTION("super-early-file-end") {
        const auto* contents = R"(
1
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("early-file-end") {
        const auto* contents = R"(
-100 1 0 0 1
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("negative_id") {
        const auto* contents = R"(
-100 1 0 0 1 0.5 -1
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("negative_id_parent") {
        const auto* contents = R"(
100 1 0 0 1 0.5 -10
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("unparseable") {
        const auto* contents = R"(
100 1 0 0 1 0.5 -10 this is some random text that isn't commented
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("repeated_id") {
        const auto* contents = R"(
1 1 0 0 1 0.5 -1
2 3 0 0 2 0.5 1
2 3 0 0 2 0.5 1 # <-- repeated id
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("unsupported_section_type") {
        const auto* contents = R"(
1 10000 0 0 1 0.5 -1
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("non_parsable") {
        const auto* contents = R"(
1 1 0 0 1 .5 "-1"
        )";
        CHECK_THROWS_AS(Morphology(contents, "swc"), RawDataError);
    }

    SECTION("soma_multi_bifurcation") {
        const auto* soma_multi_bifurcation = R"(
    1 1 0 0 1 .5 -1
    2 1 0 0 1 .5 1
    3 1 0 0 1 .5 2
    4 1 0 0 1 .5 2
        )";
        CHECK_THROWS_AS(Morphology(soma_multi_bifurcation, "swc"), SomaError);
    }

    SECTION("soma_with_neurite_parent") {
        const auto* soma_with_neurite_parent = R"(
1 2 0 0 1 .5 -1
2 1 0 0 1 .5 1
        )";
        CHECK_THROWS_AS(Morphology(soma_with_neurite_parent, "swc"), SomaError);
    }

    SECTION("self_parent") {
        const auto* self_parent = R"(
1 2 0 0 1 .5 -1
2 1 0 0 1 .5 2
        )";
        CHECK_THROWS_AS(Morphology(self_parent, "swc"), RawDataError);
    }

    SECTION("missing_parent") {
        const auto* missing_parent = R"(
1 1 0 0 1 .5 -1
2 1 0 0 1 .5 10
        )";
        CHECK_THROWS_AS(Morphology(missing_parent, "swc"), RawDataError);
    }

    SECTION("multiple_soma") {
        const auto* multiple_soma = R"(
1 1 0 0 1 .5 -1
2 1 0 0 1 .5 -1
        )";
        CHECK_THROWS_AS(Morphology(multiple_soma, "swc"), SomaError);
    }
}

TEST_CASE("morphio::swc::working") {
    SECTION("no-soma") {
        const auto* no_soma = R"(
1 2 0 0 1 .5 -1
2 2 0 0 1 .5 1
3 2 0 0 1 .5 -1
4 2 0 0 1 .5 3
        )";
        const auto m = Morphology(no_soma, "swc");

        REQUIRE(m.soma().points().empty());
        REQUIRE(m.somaType() == morphio::SomaType::SOMA_UNDEFINED);
        REQUIRE(m.diameters().size() == 4);
    }

    SECTION("chimera-axon-on-dendrite") {
        const auto* aod = R"(
1 1 0 0 1 1 -1
2 2 0 0 2 2 1
3 2 0 0 3 3 2
4 3 0 0 4 4 3
5 3 0 0 5 5 3
        )";
        const auto m = Morphology(aod, "swc");
        REQUIRE(m.sections().size() == 3);
        REQUIRE(m.diameters().size() == 6);
    }
}
