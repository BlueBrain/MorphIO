#include <morphio/enums.h>

#include <catch2/catch.hpp>
#include <sstream>


TEST_CASE("enums") {
    SECTION("SOMA_SINGLE_POINT") {
        std::stringstream ss;
        ss << morphio::enums::SomaType::SOMA_SINGLE_POINT;
        CHECK(ss.str() == "SOMA_SINGLE_POINT");
    }

    SECTION("SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS") {
        std::stringstream ss;
        ss << morphio::enums::SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
        CHECK(ss.str() == "SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS");
    }

    SECTION("SOMA_CYLINDERS") {
        std::stringstream ss;
        ss << morphio::enums::SomaType::SOMA_CYLINDERS;
        CHECK(ss.str() == "SOMA_CYLINDERS");
    }

    SECTION("SOMA_SIMPLE_CONTOUR") {
        std::stringstream ss;
        ss << morphio::enums::SomaType::SOMA_SIMPLE_CONTOUR;
        CHECK(ss.str() == "SOMA_SIMPLE_CONTOUR");
    }

    SECTION("SOMA_UNDEFINED") {
        std::stringstream ss;
        ss << morphio::enums::SomaType::SOMA_UNDEFINED;
        CHECK(ss.str() == "SOMA_UNDEFINED");
    }
}
