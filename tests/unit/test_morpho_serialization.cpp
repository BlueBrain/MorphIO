#include <sstream>

#include <boost/test/unit_test.hpp>

#include <morpho/morpho_serialization.hpp>
#include <morpho/morpho_tree.hpp>

morpho::serialization_format ALL_FORMATS[] = {
    morpho::BINARY, morpho::PORTABLE_BINARY, morpho::JSON, morpho::XML};

BOOST_AUTO_TEST_CASE(neuron_soma_serialization) {
    using namespace morpho;

    for (auto format : ALL_FORMATS) {
        std::stringstream ss;
        // serialize data in a stringstream
        {
            morpho_tree tree;
            const std::shared_ptr<morpho_node> node(
                new neuron_soma(point(0, 0, 0), 0.5));
            tree.add_node(-1, node);
            serialize(tree, ss, format);
        }
        // deserialize the stringstream and check morpho_tree
        const std::string data = ss.str();
        {
            std::stringstream iss(data);
            morpho_tree tree = deserialize(iss, format);
            BOOST_CHECK_EQUAL(tree.get_tree_size(), 1);
            const morpho_node& first_node = tree.get_node(0);
            const neuron_soma& soma =
                static_cast<const neuron_soma&>(first_node);
            BOOST_CHECK_EQUAL(soma.get_line_loop().size(), 1);
            auto sphere_soma = soma.get_sphere();
            sphere ref_sphere(point(0, 0, 0), 0.5);
            BOOST_CHECK_EQUAL(ref_sphere.get_radius(),
                              sphere_soma.get_radius());
            BOOST_CHECK(
                ref_sphere.get_center().close_to(sphere_soma.get_center()));
            BOOST_CHECK(soma.get_section_type() == neuron_struct_type::soma);
            BOOST_CHECK_EQUAL(tree.get_children(0).size(), 0);
        }
    }
}

BOOST_AUTO_TEST_CASE(neuron_section_serialization) {
    using namespace morpho;

    for (auto format : ALL_FORMATS) {
        std::stringstream ss;
        {
            morpho_tree tree;
            const std::shared_ptr<morpho_node> section(
                new neuron_section(neuron_struct_type::dentrite_basal,
                                  {point(0.1, 0.2, 0.3), point(0.4, 0.5, 0.6),
                                   point(0.7, 0.8, 0.9)},
                                  {0.15, 0.25, 0.35}));
            tree.add_node(-1, section);
            serialize(tree, ss, format);
        }

        const std::string data = ss.str();
        {
            std::stringstream iss(data);
            morpho_tree tree = deserialize(iss, format);
            BOOST_CHECK_EQUAL(tree.get_tree_size(), 1);
            const morpho_node& first_node = tree.get_node(0);
            const neuron_section& section =
                static_cast<const neuron_section&>(first_node);
            const auto radius = section.get_radius();
            BOOST_CHECK_EQUAL(radius.size(), 3);
            BOOST_CHECK_EQUAL(radius[0], 0.15);
            BOOST_CHECK_EQUAL(radius[1], 0.25);
            BOOST_CHECK_EQUAL(radius[2], 0.35);
            const auto points = section.get_points();
            BOOST_CHECK_EQUAL(points.size(), 3);
            BOOST_CHECK(points[0].close_to(point(0.1, 0.2, 0.3)));
            BOOST_CHECK(points[1].close_to(point(0.4, 0.5, 0.6)));
            BOOST_CHECK(points[2].close_to(point(0.7, 0.8, 0.9)));
        }
    }
}