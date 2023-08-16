/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/exceptions.h>
#include <morphio/morphology.h>

#include <catch2/catch.hpp>

using namespace morphio;


TEST_CASE("morphio::swc") {
    /*
    SECTION("negative_id") {
        const auto* negative_id = R"(
-100 1 0 0 1 0.5 -1
        )";
        CHECK_THROWS_AS(Morphology(negative_id, "swc"), RawDataError);
    }
    */

    SECTION("repeated_id") {
        const auto* repeated_id = R"(
1 1 0 0 1 0.5 -1
2 3 0 0 2 0.5 1
2 3 0 0 2 0.5 1 # <-- repeated id
        )";
        CHECK_THROWS_AS(Morphology(repeated_id, "swc"), RawDataError);
    }

    SECTION("unsupported_section_type") {
        const auto* unsupported_section_type = R"(
1 10000 0 0 1 0.5 -1
        )";
        CHECK_THROWS_AS(Morphology(unsupported_section_type, "swc"), RawDataError);
    }

    SECTION("non_parsable") {
        const auto* non_parsable = R"(
1 1 0 0 1 .5 "-1"
        )";
        CHECK_THROWS_AS(Morphology(non_parsable, "swc"), RawDataError);
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
