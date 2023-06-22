#include <morphio/properties.h>

#include <catch2/catch.hpp>
#include <sstream>
#include <sstream>


TEST_CASE("morphio::PointLevel") {
    using namespace morphio::Property;

    SECTION("mismatch of points and diameters"){
        std::vector<Point::Type> points;
        std::vector<Diameter::Type> diameters {1, 1};
        std::vector<Perimeter::Type> perimeters;

        CHECK_THROWS(PointLevel(points, diameters, perimeters));
    }

    SECTION("mismatch of perimeters"){
        std::vector<Point::Type> points;
        std::vector<Diameter::Type> diameters;
        std::vector<Perimeter::Type> perimeters {1, 1};

        CHECK_THROWS(PointLevel(points, diameters, perimeters));
    }

    SECTION("ostream"){
        PointLevel p;
        PointLevel pl;
        p = pl;

        std::stringstream s0;
        std::stringstream s1;

        s0 << p;
        s1 << pl;

        CHECK(s0.str() == s1.str());
    }
}
