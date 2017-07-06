/*
 * Copyright (C) 2017 Tristan Carel <tristan.carel@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#define BOOST_TEST_MODULE testSWC
#define BOOST_TEST_MAIN

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <morpho/morpho_circuit.hpp>
#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_reader.hpp>
#include <morpho/morpho_stats.hpp>
#include <morpho/morpho_transform.hpp>
#include <morpho/morpho_transform_filters.hpp>
#include <src/morpho/gmsh_exporter.hpp>

BOOST_AUTO_TEST_CASE(test_swc_parser) {
    using namespace morpho;
    using namespace boost::filesystem;
    using hadoken::format::scat;

    const auto swc_file = path(SWC_MORPHO_TEST_FILE);
    const auto h5_file =
        path(MORPHO_OUTPUT_DIR) / swc_file.stem().concat(".h5");

    const morpho_tree swc_tree = reader::create_morpho_tree(swc_file.string());

    { // write
        h5_v1::morpho_writer writer(h5_file.string());
        writer.write(swc_tree);
    }

    { // read back

        const morpho_tree h5_tree =
            reader::create_morpho_tree(h5_file.string());

        std::size_t n_section1 = stats::total_number_sectiones(swc_tree);
        std::size_t n_section2 = stats::total_number_sectiones(h5_tree);

        BOOST_CHECK_EQUAL(n_section1, n_section2);

        std::size_t n_points1 = stats::total_number_point(swc_tree);
        std::size_t n_points2 = stats::total_number_point(h5_tree);

        BOOST_CHECK_EQUAL(n_points1, n_points2);

        double median_radius1 = stats::median_radius_segment(swc_tree);
        double median_radius2 = stats::median_radius_segment(h5_tree);

        BOOST_CHECK_EQUAL(median_radius1, median_radius2);
    }
}

BOOST_AUTO_TEST_CASE(test_swc_to_gmsh) {
    using namespace morpho;
    using namespace boost::filesystem;
    using hadoken::format::scat;

    const auto swc_file = path(SWC_MORPHO_TEST_FILE);
    const auto geo_file =
        path(MORPHO_OUTPUT_DIR) / swc_file.stem().concat(".geo");

    gmsh_exporter::exporter_flags export_flags = 0;

    std::vector<morpho_tree> trees = {
        morpho::reader::create_morpho_tree(swc_file.string())};

    std::for_each(trees.begin(), trees.end(), [&](morpho_tree& tree) {
        morpho_tree final_tree = morpho_transform(
            tree, {
                      std::make_shared<delete_duplicate_point_operation>(),
                      std::make_shared<duplicate_first_point_operation>(),
                  });
        tree.swap(final_tree);
    });

    gmsh_exporter exporter(std::move(trees), geo_file.string(), export_flags);
    exporter.set_identifier(std::string("morphology: ") + geo_file.string());
    exporter.export_to_wireframe();
}
