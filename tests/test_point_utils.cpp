#include "../src/point_utils.h"

#include <catch2/catch.hpp>
#include <sstream>      // std::stringstream


TEST_CASE("morphio::Point") {
    using namespace morphio;
    Point p{10., 10., 10.};
    std::vector<Point> points = {p, p, p};

    CHECK(subtract(p, p) == morphio::Point{0, 0, 0});
    CHECK(euclidean_distance(p, p) == 0);
    CHECK(dumpPoint(p) == "10 10 10");
    CHECK(dumpPoints(points) == "10 10 10\n10 10 10\n10 10 10\n");

    {
        std::stringstream ss;
        ss << p;
        CHECK(ss.str() == "10 10 10");
    }

    {
        std::stringstream ss;
        ss << points;
        CHECK(ss.str() == "10 10 10\n10 10 10\n10 10 10\n");
    }

    {
        auto r = morphio::range<const morphio::Point>();
        std::stringstream ss;
        ss << r;
        CHECK(ss.str() == "");

        ss.clear();

        r = morphio::range<const morphio::Point>(points).subspan(0, 2);
        ss << r;
        CHECK(ss.str() == "10 10 10\n10 10 10\n");
    }
}
