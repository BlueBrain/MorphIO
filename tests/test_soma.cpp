/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/enums.h>
#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/soma.h>

#include <catch2/catch.hpp>
#include <sstream>


TEST_CASE("morphio::soma") {
    using namespace morphio::Property;

    SECTION("mismatch of points and diameters") {
        std::vector<Point::Type> points;
        std::vector<Diameter::Type> diameters{1, 1};
        std::vector<Perimeter::Type> perimeters;

        CHECK_THROWS(PointLevel(points, diameters, perimeters));
    }

    SECTION("Volumes") {
        const auto three_point = morphio::Morphology("data/three_point_soma.swc");
        CHECK_THAT(three_point.soma().volume(), Catch::WithinAbs(50.26, 0.01));

        const auto soma_cylinders = morphio::Morphology("data/soma_cylinders.swc");
        CHECK_THROWS(soma_cylinders.soma().volume());

        const auto soma_single_point = morphio::Morphology("data/soma_single_frustum.swc");
        CHECK_THROWS(soma_single_point.soma().volume());

        const auto soma_simple_contour = morphio::Morphology("data/simple.asc");
        CHECK_THROWS(soma_simple_contour.soma().volume());
    }

    SECTION("print SomaTypes") {
        using morphio::SomaType;
        std::stringstream ss;

        ss << SomaType::SOMA_UNDEFINED;
        CHECK(ss.str() == "SOMA_UNDEFINED");

        ss.str("");
        ss << SomaType::SOMA_SINGLE_POINT;
        CHECK(ss.str() == "SOMA_SINGLE_POINT");

        ss.str("");
        ss << SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
        CHECK(ss.str() == "SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS");

        ss.str("");
        ss << SomaType::SOMA_CYLINDERS;
        CHECK(ss.str() == "SOMA_CYLINDERS");

        ss.str("");
        ss << SomaType::SOMA_SIMPLE_CONTOUR;
        CHECK(ss.str() == "SOMA_SIMPLE_CONTOUR");
    }
}
