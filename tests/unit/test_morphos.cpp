#define BOOST_TEST_MODULE testMorpho
#define BOOST_TEST_MAIN

#include <iostream>
#include <memory>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_spatial.hpp>
#include <morpho/morpho_tree.hpp>

#include <morpho/morpho_reader.hpp>

BOOST_AUTO_TEST_CASE(test_morpho_tree) {
    using namespace morpho;

    morpho_tree tree;
    std::size_t id;

    BOOST_CHECK_EQUAL(tree.get_tree_size(), 0);

    std::shared_ptr<morpho_node> node(new neuron_soma(point(0, 0, 0), 0.5));

    id = tree.add_node(-1, node);

    BOOST_CHECK_EQUAL(id, 0);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 1);

    const morpho_node& first_node = tree.get_node(0);
    BOOST_CHECK_EQUAL(first_node.is_of_type(morpho_node_type::neuron_soma_type),
                      true);
    BOOST_CHECK_EQUAL(
        first_node.is_of_type(morpho_node_type::neuron_node_3d_type), true);
    BOOST_CHECK_EQUAL(
        first_node.is_of_type(morpho_node_type::neuron_section_type), false);

    const neuron_soma& soma = static_cast<const neuron_soma&>(first_node);
    BOOST_CHECK_EQUAL(soma.get_line_loop().size(), 1);

    auto sphere_soma = soma.get_sphere();
    sphere ref_sphere(point(0, 0, 0), 0.5);
    BOOST_CHECK_EQUAL(ref_sphere.get_radius(), sphere_soma.get_radius());
    BOOST_CHECK(ref_sphere.get_center().close_to(sphere_soma.get_center()));

    BOOST_CHECK(soma.get_section_type() == neuron_struct_type::soma);

    BOOST_CHECK_EQUAL(tree.get_children(0).size(), 0);

    std::shared_ptr<morpho_node> node2(new neuron_section(
        neuron_struct_type::dentrite_basal,
        {point(0.1, 0.1, 0.1), point(0.2, 0.2, 0.2)}, {0.5, 0.5}));

    id = tree.add_node(0, node2);

    const morpho_node& second_node = tree.get_node(1);
    BOOST_CHECK_EQUAL(id, 1);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 2);
    BOOST_CHECK_EQUAL(
        second_node.is_of_type(morpho_node_type::neuron_section_type), true);
    BOOST_CHECK_EQUAL(
        second_node.is_of_type(morpho_node_type::neuron_node_3d_type), true);
    BOOST_CHECK_EQUAL(
        second_node.is_of_type(morpho_node_type::neuron_soma_type), false);

    const neuron_section& section1 =
        static_cast<const neuron_section&>(second_node);
    BOOST_CHECK(section1.get_section_type() ==
                neuron_struct_type::dentrite_basal);

    BOOST_CHECK_EQUAL(tree.get_children(0).size(), 1);
    BOOST_CHECK_EQUAL(tree.get_children(0)[0], id);

    BOOST_CHECK_EQUAL(tree.get_parent(1), 0);
    BOOST_CHECK_EQUAL(tree.get_parent(0), -1);

    // test swap method
    morpho_tree tree_copy;
    BOOST_CHECK_EQUAL(tree_copy.get_tree_size(), 0);

    tree_copy.swap(tree);
    BOOST_CHECK_EQUAL(tree_copy.get_tree_size(), 2);
    BOOST_CHECK_EQUAL(tree.get_tree_size(), 0);
}

BOOST_AUTO_TEST_CASE(test_load_h5v1) {
    using namespace morpho;

    // load h5 file
    const morpho_tree tree = reader::create_morpho_tree(H5V1_MORPHO_TEST_FILE);

    {
        h5_v1::morpho_reader reader(H5V1_MORPHO_TEST_FILE);

        auto reader_structs = reader.get_struct_raw();
        auto raw_points = reader.get_points_raw();

        BOOST_CHECK_EQUAL(reader_structs.size1(), tree.get_tree_size());

        BOOST_CHECK_EQUAL(
            tree.get_node(0).is_of_type(morpho_node_type::neuron_soma_type),
            true);
        const neuron_soma& soma =
            static_cast<const neuron_soma&>(tree.get_node(0));
        BOOST_CHECK(soma.get_section_type() == neuron_struct_type::soma);

        BOOST_CHECK_EQUAL(soma.get_line_loop().size(),
                          reader.get_section_range_raw(0).second);

        for (std::size_t i = 1; i < tree.get_tree_size(); ++i) {
            BOOST_CHECK_EQUAL(tree.get_node(i).is_of_type(
                                  morpho_node_type::neuron_section_type),
                              true);

            const neuron_section& nsection =
                static_cast<const neuron_section&>(tree.get_node(i));

            const std::size_t section_raw_offset =
                reader.get_section_range_raw(i).first;
            const std::size_t section_raw_size =
                reader.get_section_range_raw(i).second;
            BOOST_CHECK_EQUAL(nsection.get_number_points(), section_raw_size);
            BOOST_CHECK_EQUAL(nsection.get_points().size(), section_raw_size);
            BOOST_CHECK_EQUAL(nsection.get_radius().size(), section_raw_size);

            BOOST_CHECK(nsection.get_section_type() == neuron_struct_type::axon ||
                        nsection.get_section_type() ==
                            neuron_struct_type::dentrite_basal ||
                        nsection.get_section_type() ==
                            neuron_struct_type::dentrite_apical);

            for (std::size_t j = 0; j < nsection.get_number_points(); ++j) {
                const auto& points = nsection.get_points();
                const auto& radius = nsection.get_radius();

                point point_raw(raw_points(section_raw_offset + j, 0),
                                raw_points(section_raw_offset + j, 1),
                                raw_points(section_raw_offset + j, 2));
                double radius_raw = raw_points(section_raw_offset + j, 3) / 2;

                BOOST_CHECK(point_raw.close_to(points[j]));
                BOOST_CHECK(hadoken::math::close_to_abs(radius_raw, radius[j]));
            }
        }
    }
}

template <typename Fun>
void for_each_point_section(const morpho::morpho_node& node, Fun f) {
    using namespace morpho;
    if (node.is_of_type(morpho_node_type::neuron_soma_type)) {
        const neuron_soma& b = static_cast<const neuron_soma&>(node);
        const auto& points = b.get_line_loop();
        std::for_each(points.begin(), points.end(), f);
    } else if (node.is_of_type(morpho_node_type::neuron_section_type)) {
        const neuron_section& b = static_cast<const neuron_section&>(node);
        const auto& points = b.get_points();
        std::for_each(points.begin(), points.end(), f);
    } else {
        throw std::invalid_argument("Invalid type");
    }
}

template <typename Fun>
void for_each_point_tree(const morpho::morpho_tree& tree, Fun f) {
    using namespace morpho;
    for (std::size_t i = 0; i < tree.get_tree_size(); ++i) {
        const morpho_node& node = tree.get_node(i);
        for_each_point_section<Fun>(node, f);
    }
}

// same but ignore soma
template <typename Fun>
void for_each_point_section_tree(const morpho::morpho_tree& tree, Fun f) {
    using namespace morpho;
    for (std::size_t i = 1; i < tree.get_tree_size(); ++i) {
        const morpho_node& node = tree.get_node(i);
        for_each_point_section<Fun>(node, f);
    }
}

bool is_inside_box(const morpho::box& b, const morpho::point& p) {
    using namespace morpho;
    const point b_min = b.min_corner();
    const point b_max = b.max_corner();

    for (std::size_t i = 0; i < 3; ++i) {
        if (b_min(i) > p(i))
            return false;

        if (b_max(i) < p(i))
            return false;
    }

    return true;
}

BOOST_AUTO_TEST_CASE(test_bounding_box) {
    using namespace morpho;
    // load h5 file
    const morpho_tree tree = reader::create_morpho_tree(H5V1_MORPHO_TEST_FILE);

    const box global_tree_box = tree.get_bounding_box();
    std::cout << "global tree bounding box " << global_tree_box.min_corner()
              << " " << global_tree_box.max_corner() << std::endl;

    for_each_point_tree(tree, [&](const point& p) {
        BOOST_CHECK(is_inside_box(global_tree_box, p) == true);
    });

    for (std::size_t i = 0; i < tree.get_tree_size(); ++i) {
        const morpho_node& node = tree.get_node(i);
        box node_box = node.get_bounding_box();
        std::cout << "section bounding box " << node_box.min_corner() << " "
                  << node_box.max_corner() << std::endl;

        for_each_point_section(node, [&](const point& p) {

            BOOST_CHECK(is_inside_box(node_box, p));
        });
    }
}

BOOST_AUTO_TEST_CASE(test_spatial) {
    using namespace morpho;
    // load h5 file
    const std::shared_ptr<morpho_tree> tree(
        new morpho_tree(reader::create_morpho_tree(H5V1_MORPHO_TEST_FILE)));

    spatial_index index;

    index.add_morpho_tree(tree);

    for_each_point_section_tree(*tree, [&](const point& p) {
        BOOST_CHECK(index.is_within(p) == true);
    });
}


//Auto detection of cell type from metadata
BOOST_AUTO_TEST_CASE(test_cell_type) {
    using namespace morpho;

    const std::shared_ptr<morpho_tree> tree(
        new morpho_tree(reader::create_morpho_tree(ASTROCITE_MORPHO_TEST_FILE)));

    //Need to check
    BOOST_CHECK(tree->get_cell_type() == morpho::cell_family::GLIA);
}
