#include <morphio/enums.h>
#include <morphio/properties.h>

#include <catch2/catch.hpp>
#include <sstream>
#include <vector>


TEST_CASE("morphio::PointLevel") {
    using namespace morphio::Property;

    SECTION("mismatch of points and diameters") {
        std::vector<Point::Type> points;
        std::vector<Diameter::Type> diameters{1, 1};
        std::vector<Perimeter::Type> perimeters;

        CHECK_THROWS(PointLevel(points, diameters, perimeters));
    }

    SECTION("mismatch of perimeters") {
        std::vector<Point::Type> points;
        std::vector<Diameter::Type> diameters;
        std::vector<Perimeter::Type> perimeters{1, 1};

        CHECK_THROWS(PointLevel(points, diameters, perimeters));
    }

    SECTION("ostream") {
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

/* TEST_CASE("morphio::MitochondrialPointLevel") { */
/*     using namespace morphio::Property; */

/*     const auto mito = MitochondriaPointLevel{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2, 3}}; */
/*     const auto mito2 = MitochondriaPointLevel{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2, 3}}; */
/*     const auto mito3 = MitochondriaPointLevel{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2}}; */

/*     SECTION("mismatch of points and diameters") { */
/*         std::vector<Point::Type> points; */
/*         std::vector<Diameter::Type> diameters{1, 1}; */
/*         std::vector<Perimeter::Type> perimeters; */
/*         std::vector<Volume::Type> volumes{1, 1}; */

/*         CHECK_THROWS(MitochondriaPointLevel(points, diameters, perimeters, volumes)); */
/*     } */

/*     SECTION("mismatch of perimeters") { */
/*         std::vector<Point::Type> points; */
/*         std::vector<Diameter::Type> diameters; */
/*         std::vector<Perimeter::Type> perimeters{1, 1}; */
/*         std::vector<Volume::Type> volumes{1, 1}; */

/*         CHECK_THROWS(MitochondriaPointLevel(points, diameters, perimeters, volumes)); */
/*     } */

/*     SECTION("mismatch of volumes") { */
/*         std::vector<Point::Type> points; */
/*         std::vector<Diameter::Type> diameters; */
/*         std::vector<Perimeter::Type> perimeters; */
/*         std::vector<Volume::Type> volumes{1, 1}; */

/*         CHECK_THROWS(MitochondriaPointLevel(points, diameters, perimeters, volumes)); */
/*     } */

/*     SECTION("ostream") { */
/*         MitochondriaPointLevel p; */
/*         MitochondriaPointLevel pl; */
/*         p = pl; */

/*         std::stringstream s0; */
/*         std::stringstream s1; */

/*         s0 << p; */
/*         s1 << pl; */

/*         CHECK(s0.str() == s1.str()); */
/*     } */

/*     SECTION("compare") { */
/*         CHECK(mito == mito2); */
/*         CHECK(mito != mito3); */
/*     } */
/* } */

/* TEST_CASE("morphio::PointLevel::compare") { */
/*     using namespace morphio::Property; */

/*     const auto sections = std::vector<Section::Type>{{0, 0}, {1, 0}, {2, 0}, {3, 0}}; */
/*     const auto sectionTypes = std::vector<SectionType::Type>{ */
/*         morphio::SECTION_AXON, morphio::SECTION_AXON, morphio::SECTION_AXON, morphio::SECTION_AXON}; */
/*     const auto children = std::map<int, std::vector<unsigned int>>{{0, {1, 2, 3}}}; */
/*     auto sl0 = SectionLevel{sections, sectionTypes, children}; */


/*     SECTION("self compare") { */
/*         CHECK(sl0 == sl0); */
/*     } */

/*     SECTION("different types") { */
/*         auto sl1 = SectionLevel{sections, { */
/*             morphio::SECTION_DENDRITE, morphio::SECTION_AXON, morphio::SECTION_AXON, morphio::SECTION_AXON}, children}; */

/*         CHECK(sl0 != sl1); */
/*         CHECK(sl0.diff(sl1, morphio::enums::LogLevel::DEBUG)); */
/*     } */

/*     SECTION("different lenghts") { */
/*         auto sl1 = SectionLevel{{}, {}, {}}; */
/*         CHECK(sl0 != sl1); */
/*         CHECK(sl0.diff(sl1, morphio::enums::LogLevel::DEBUG)); */
/*     } */
/* } */

/* TEST_CASE("morphio::CellLevel::compare") { */
/*     using namespace morphio::Property; */

/*     const auto sections = std::vector<Section::Type>{{0, 0}, {1, 0}, {2, 0}, {3, 0}}; */
/*     const auto sectionTypes = std::vector<SectionType::Type>{ */
/*         morphio::SECTION_AXON, morphio::SECTION_AXON, morphio::SECTION_AXON, morphio::SECTION_AXON}; */
/*     const auto children = std::map<int, std::vector<unsigned int>>{{0, {1, 2, 3}}}; */
/*     auto sl0 = SectionLevel{sections, sectionTypes, children}; */

/*     auto cl0 = CellLevel{{}, {}, sl0, {}, {}, {}, {}}; */

/*     SECTION("self compare") { */
/*         CHECK(cl0 == cl0); */
/*     } */

/*     SECTION("different types") { */
/*         auto cl1 = CellLevel{{}, {}, sl0, {}, {}, {}, {}}; */
/*         CHECK(cl0 != cl1); */
/*         CHECK(cl0.diff(cl1, morphio::enums::LogLevel::DEBUG)); */
/*     } */

/*     SECTION("different lenghts") { */
/*         auto cl1 = CellLevel{{}, {}, {}, {}, {}, {}}; */
/*         CHECK(cl0 != cl1); */
/*         CHECK(cl0.diff(cl1, morphio::enums::LogLevel::DEBUG)); */
/*     } */

/* } */
