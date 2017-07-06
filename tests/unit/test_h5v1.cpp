#define BOOST_TEST_MODULE testH5v1
#define BOOST_TEST_MAIN

#include <iostream>
#include <memory>
#include <sstream>

#include <boost/test/unit_test.hpp>

#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_reader.hpp>
#include <morpho/morpho_stats.hpp>
#include <morpho/morpho_transform.hpp>
#include <morpho/morpho_transform_filters.hpp>
#include <morpho/morpho_tree.hpp>

BOOST_AUTO_TEST_CASE(test_h5v1_rw) {
    using namespace morpho;
    const std::string testfile_write("testfile_write_h5v1.h5");

    morpho_tree tree1, tree2;

    { // read
        tree1 = reader::create_morpho_tree(H5V1_MORPHO_TEST_FILE);
    }

    { // write

        h5_v1::morpho_writer writer(testfile_write);
        writer.write(tree1);
    }

    { // read back

        tree2 = reader::create_morpho_tree(testfile_write);

        std::size_t n_section1 = stats::total_number_sectiones(tree1);
        std::size_t n_section2 = stats::total_number_sectiones(tree2);

        BOOST_CHECK_EQUAL(n_section1, n_section2);

        std::size_t n_points1 = stats::total_number_point(tree1);
        std::size_t n_points2 = stats::total_number_point(tree2);

        BOOST_CHECK_EQUAL(n_points1, n_points2);

        double median_radius1 = stats::median_radius_segment(tree1);
        double median_radius2 = stats::median_radius_segment(tree2);

        BOOST_CHECK_EQUAL(median_radius1, median_radius2);
    }
}

BOOST_AUTO_TEST_CASE(test_h5v1_test_filter_duplicated) {
    using namespace morpho;
    const std::string testfile_write("test_filter_dup.h5");

    morpho_tree tree1, tree2;

    tree1 = reader::create_morpho_tree(H5V1_MORPHO_TEST_FILE);

    morpho_operation_chain chain;
    chain.push_back(std::make_shared<delete_duplicate_point_operation>());

    tree2 = morpho_transform(tree1, chain);

    std::size_t n_section1 = stats::total_number_sectiones(tree1);
    std::size_t n_section2 = stats::total_number_sectiones(tree2);

    BOOST_CHECK_EQUAL(n_section1, n_section2);

    std::size_t n_points1 = stats::total_number_point(tree1);
    std::size_t n_points2 = stats::total_number_point(tree2);

    BOOST_CHECK_EQUAL(n_points1, 3334);
    BOOST_CHECK_EQUAL(n_points2, 3243);
}
