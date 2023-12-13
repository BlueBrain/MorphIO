/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
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
        std::vector<Point::Type> points{{0, 0, 0}, {1, 1, 1}};
        std::vector<Diameter::Type> diameters{1, 1};
        std::vector<Perimeter::Type> perimeters{1, 1};

        PointLevel p{points, diameters, perimeters};
        PointLevel pl{points, diameters, perimeters};

        p = pl;

        std::stringstream s0;
        std::stringstream s1;

        s0 << p;
        s1 << pl;

        CHECK(s0.str() == s1.str());
    }
}

TEST_CASE("morphio::SectionLevel") {
    using namespace morphio::Property;
    const auto sections = std::vector<Section::Type>{{0, 0}, {1, 0}, {2, 0}, {3, 0}};
    const auto sectionTypes = std::vector<SectionType::Type>{morphio::SECTION_AXON,
                                                             morphio::SECTION_AXON,
                                                             morphio::SECTION_AXON,
                                                             morphio::SECTION_AXON};
    const auto children = std::map<int, std::vector<unsigned int>>{{0, {1, 2, 3}}};
    auto sl0 = SectionLevel{sections, sectionTypes, children};


    SECTION("self compare") {
        CHECK(sl0 == sl0);
    }

    SECTION("different types") {
        auto sl1 = SectionLevel{sections,
                                {morphio::SECTION_DENDRITE,
                                 morphio::SECTION_AXON,
                                 morphio::SECTION_AXON,
                                 morphio::SECTION_AXON},
                                children};

        CHECK(sl0 != sl1);
        CHECK(sl0.diff(sl1));
    }

    SECTION("different lenghts") {
        auto sl1 = SectionLevel{{}, {}, {}};
        CHECK(sl0 != sl1);
        CHECK(sl0.diff(sl1));
    }
}


TEST_CASE("morphio::CellLevel::compare") {
    using namespace morphio::Property;

    CellLevel cl0;

    SECTION("self compare") {
        CHECK(cl0 == cl0);
        CHECK(!(cl0 != cl0));
    }

    SECTION("different cell families") {
        auto cl1 = CellLevel{{}, morphio::enums::GLIA, {}, {}, {}};
        CHECK(cl0 != cl1);
        CHECK(cl0.diff(cl1));
    }

    SECTION("different soma types") {
        auto cl1 =
            CellLevel{{}, morphio::enums::NEURON, morphio::enums::SomaType::SOMA_CYLINDERS, {}, {}};
        CHECK(cl0 != cl1);
        CHECK(cl0.diff(cl1));
    }
}


TEST_CASE("morphio::MitochondriaPointLevel") {
    using namespace morphio::Property;

    SECTION("mismatch of section_ids and relativePathLengths") {
        std::vector<MitoNeuriteSectionId::Type> sectionIds{0};
        std::vector<MitoPathLength::Type> relativePathLengths;
        std::vector<MitoDiameter::Type> diameters;

        CHECK_THROWS(MitochondriaPointLevel(sectionIds, relativePathLengths, diameters));
    }

    SECTION("mismatch of diameters") {
        std::vector<MitoNeuriteSectionId::Type> sectionIds;
        std::vector<MitoPathLength::Type> relativePathLengths;
        std::vector<MitoDiameter::Type> diameters{1};

        CHECK_THROWS(MitochondriaPointLevel(sectionIds, relativePathLengths, diameters));
    }

    SECTION("compare") {
        std::vector<MitoNeuriteSectionId::Type> sectionIds;
        std::vector<MitoPathLength::Type> relativePathLengths;
        std::vector<MitoDiameter::Type> diameters;

        auto mpl0 = MitochondriaPointLevel(sectionIds, relativePathLengths, diameters);

        CHECK(mpl0 == mpl0);
        CHECK(!(mpl0 != mpl0));
    }
}

TEST_CASE("morphio::MitochondriaSectionLevel") {
    using namespace morphio::Property;

    std::vector<Section::Type> sections{morphio::SECTION_AXON};
    std::map<int, std::vector<unsigned int>> children;

    auto sl0 = MitochondriaSectionLevel{sections, children};


    SECTION("self compare") {
        CHECK(sl0 == sl0);
    }

    SECTION("different types") {
        std::vector<Section::Type> sections1{morphio::SECTION_DENDRITE};
        auto sl1 = MitochondriaSectionLevel{sections1, children};

        CHECK(sl0 != sl1);
        CHECK(sl0.diff(sl1));
    }

    SECTION("different lenghts") {
        auto sl1 = MitochondriaSectionLevel{{}, {}};
        CHECK(sl0 != sl1);
        CHECK(sl0.diff(sl1));
    }
}
