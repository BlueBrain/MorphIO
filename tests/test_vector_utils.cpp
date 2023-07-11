/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/vector_types.h>

#include <catch2/catch.hpp>

TEST_CASE("VectorTypes.Point", "[vector_types]") {
    using morphio::Point;
    const Point zero{0., 0., 0.};

    SECTION("operator==") {
        CHECK(zero == zero);
    }
    SECTION("operator+") {
        // CHECK(zero == zero);
    }
    SECTION("operator-") {}
    SECTION("operator*") {
        // try pre and post versions
    }
    SECTION("operator/") {}

    SECTION("operator+=") {}
    SECTION("operator-=") {}
    SECTION("operator/=") {}
}

/*
template <typename T>
Point centerOfGravity(const T& points);
template <typename T>
floatType maxDistanceToCenterOfGravity(const T& points);

    REQUIRE_THAT(soma.surface(), Catch::WithinAbs(1017.87604, 0.001));
*/
