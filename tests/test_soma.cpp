
#include <cmath>
#include <catch2/catch.hpp>

#include <morphio/soma.h>
#include <morphio/enums.h>

#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>


TEST_CASE("soma-default-constructor", "[soma]") {

    const morphio::Soma soma;

    REQUIRE(soma.type() == morphio::enums::SomaType::SOMA_UNDEFINED);
    REQUIRE(soma.points().empty());
    REQUIRE(soma.diameters().empty());

    REQUIRE(soma.properties()._points.empty());
    REQUIRE(soma.properties()._diameters.empty());
    REQUIRE(soma.properties()._perimeters.empty());

    morphio::Point center = soma.center();

    REQUIRE(std::isnan(center[0]));
    REQUIRE(std::isnan(center[1]));
    REQUIRE(std::isnan(center[2]));
    REQUIRE(std::isnan(soma.surface()));
    REQUIRE(std::isnan(soma.volume()));
    REQUIRE(std::isnan(soma.maxDistance()));
}


TEST_CASE("soma-point-properties-constructor", "[soma]") {

    auto properties = morphio::Property::PointLevel();

    properties._points = {{0., 1., 2.}, {3., 4., 5.}};
    properties._diameters = {0.2, 0.3};

    morphio::Soma soma(properties);

    REQUIRE(soma.points() == properties._points);
    REQUIRE(soma.diameters() == properties._diameters);
    REQUIRE(soma.type() == morphio::enums::SomaType::SOMA_UNDEFINED);

    REQUIRE(soma.center() == morphio::Point({1.5, 2.5, 3.5}));
    REQUIRE_THAT(soma.maxDistance(), Catch::WithinAbs(2.598076, 1e-6));
}


TEST_CASE("soma-properties-constructor", "[soma]") {

    auto properties = morphio::Property::Properties();

    std::vector<morphio::Point> expected_points = {{0., 1., 2.}, {3., 4., 5.}, {6., 7., 8.}};
    std::vector<morphio::floatType> expected_diameters = {0.2, 0.3, 0.4};
    morphio::enums::SomaType expected_soma_type = morphio::enums::SomaType::SOMA_SIMPLE_CONTOUR;

    properties._somaLevel._points = expected_points;
    properties._somaLevel._diameters = expected_diameters;
    properties._cellLevel._somaType = expected_soma_type;

    morphio::Soma soma(properties);

    REQUIRE(soma.points() == expected_points);
    REQUIRE(soma.diameters() == expected_diameters);
    REQUIRE(soma.type() == expected_soma_type);

}


TEST_CASE("soma-copy-constructor", "[soma]") {

    auto properties = morphio::Property::PointLevel();

    properties._points = {{0., 1., 2.}, {3., 4., 5.}};
    properties._diameters = {0.2, 0.3};

    // allocate a soma in the heap to delete it afterwards
    // and check if the copy maintains its data
    morphio::Soma* soma = new morphio::Soma(properties);
    morphio::Soma soma_copy(*soma);

    auto expected_points = soma->points();
    auto expected_diameters =  soma->diameters();
    auto expected_type = soma->type();

    REQUIRE(expected_points == soma_copy.points());
    REQUIRE(expected_diameters == soma_copy.diameters());
    REQUIRE(expected_type == soma_copy.type());

    delete soma;

    REQUIRE(expected_points == soma_copy.points());
    REQUIRE(expected_diameters == soma_copy.diameters());
    REQUIRE(expected_type == soma_copy.type());

}


TEST_CASE("soma-immutable-morphology-constructor", "[soma]") {

    const auto soma = morphio::Morphology("data/h5/v1/Neuron.h5").soma();

    REQUIRE(soma.type() == morphio::SomaType::SOMA_SIMPLE_CONTOUR);

    std::vector<morphio::Point> expected_soma_points = {
        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.2f, 0.0f }, { 0.1f, 0.1f, 0.0f }
    };
    REQUIRE(soma.points() == expected_soma_points);

    std::vector<morphio::floatType> expected_soma_diameters = { 0.2f, 0.2f, 0.2f };
    REQUIRE(soma.diameters() == expected_soma_diameters);


}


TEST_CASE("soma-mutable-morphology-constructor", "[soma]") {

    auto morph = morphio::mut::Morphology("data/h5/v1/Neuron.h5");

    const auto soma = *morph.soma();

    REQUIRE(soma.type() == morphio::SomaType::SOMA_SIMPLE_CONTOUR);

    std::vector<morphio::Point> expected_soma_points = {
        { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.2f, 0.0f }, { 0.1f, 0.1f, 0.0f }
    };
    REQUIRE(soma.points() == expected_soma_points);

    std::vector<morphio::floatType> expected_soma_diameters = { 0.2f, 0.2f, 0.2f };
    REQUIRE(soma.diameters() == expected_soma_diameters);

}


TEST_CASE("soma-mutable-morphology-mutation", "[soma]") {

    auto morph = morphio::mut::Morphology("data/h5/v1/Neuron.h5");

    std::vector<morphio::Point> expected_soma_points = {
        { 0.1f, 0.1f, 0.0f }
    };
    morph.soma()->points() = expected_soma_points;
    REQUIRE(morph.soma()->points() == expected_soma_points);

    std::vector<morphio::floatType> expected_soma_diameters = { 3.0f };
    morph.soma()->diameters() = expected_soma_diameters;
    REQUIRE(morph.soma()->diameters() == expected_soma_diameters);

}
