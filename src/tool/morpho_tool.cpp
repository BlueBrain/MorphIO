/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
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
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

#include <boost/geometry.hpp>

#include <hadoken/format/format.hpp>
#include <hadoken/string/algorithm.hpp>

#include <morpho/morpho_circuit.hpp>
#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_mesher.hpp>
#include <morpho/morpho_reader.hpp>
#include <morpho/morpho_stats.hpp>
#include <morpho/morpho_transform_filters.hpp>

#include "gmsh_exporter.hpp"
#include "x3d_exporter.hpp"

using namespace std;
using namespace morpho;

namespace fmt = hadoken::format;
namespace po = boost::program_options;

const std::string
    delete_duplicate_point_operation_str("delete_duplicate_point");
const std::string duplicate_first_point_operation_str("duplicate_first_point");
const std::string soma_sphere_operation_str("soma_sphere");
const std::string simplify_section_extreme_str("simplify_section_extreme");

std::string version() {
    return std::string(MORPHO_VERSION_MAJOR "." MORPHO_VERSION_MINOR);
}

po::parsed_options parse_args(int argc, char** argv, po::variables_map& res,
                              std::string& help_msg) {
    po::options_description general(
        "Commands:\n"
        "\t\t\n"
        "  stats [morphology-file]:\t morphology statistics\n"
        "  export h5v1 [morphology-file] [output-h5v1-file]:\t\texport "
        "morphology file to h5v1 format\n"
        "  export gmsh [morphology-file] [geo-file]:\t\texport morphology file "
        "to .geo file format\n"
        "  export gmsh [mvd-file] --morpho-dir [morpho_dir] [geo-file]:\t\t "
        "export entire circuit to .geo file format\n"
        "  export x3d [morphology-file] [x3d-file]:\t\texport morphology file "
        "to .x3d file format\n"
        "\n\n"
        "Options");
    general.add_options()("help", "produce a help message")(
        "version", "output the version number")(
        "transform", po::value<std::string>(),
        "h5v1,gmsh: apply a transform operation to the morphology, --transform "
        "for a list")("morpho-dir", po::value<std::string>(),
                      "circuit mode: directory to use to load morphologies")(
        "point-cloud", "gmsh: export to a point cloud")(
        "wireframe", "gmsh: export to a wired morphology (default)")(
        "3d-object", "gmsh: export to a 3D object model")(
        "with-dmg", "gmsh: export to a dmg file format as well")(
        "with-bounding-box",
        "gmsh: add a bounding box to the geometry based on neurons info")(
        "dont-pack", "gmsh: do not pack the geometrical elements by section")(
        "single-soma",
        "gmsh: represent soma as a single element, point or sphere")(
        "sphere", "x3d: export cloud of sphere (default)")(
        "error-bound", po::value<double>(), "mesh: error bound for the "
                                            "dichotomy search during meshing "
                                            "1/v (default : 100000)")(
        "command", po::value<std::string>(),
        "command to execute")("subargs", po::value<std::vector<std::string>>(),
                              "Arguments for command");
    ;

    po::positional_options_description pos;
    pos.add("command", 1).add("subargs", -1);

    auto opts = po::command_line_parser(argc, argv)
                    .options(general)
                    .positional(pos)
                    .allow_unregistered()
                    .run();
    po::store(opts, res);
    if (res.count("help")) {
        fmt::scat(std::cout, general, "\n");
        exit(0);
    }
    help_msg = fmt::scat(general);
    return opts;
}

std::shared_ptr<morpho_tree>
load_morphology(const std::string& morphology_file) {
    return std::shared_ptr<morpho_tree>(
        new morpho_tree(reader::create_morpho_tree(morphology_file)));
}

void transform_show_help() {
    fmt::scat(
        std::cout, "Invalid --transform usage \n",
        "\t Syntax: --transform=[operation1,operation2] \n", "\n"
                                                             "\t \n",
        "\t Available operations: \n", "\t\t *",
        delete_duplicate_point_operation_str,
        "*:\t remove duplicated points in every section\n", "\t\t *",
        duplicate_first_point_operation_str, "*:\t duplicate the last point of "
                                             "every section as first point of "
                                             "its children \n",
        "\t\t *", soma_sphere_operation_str,
        "*:\t transform a line-loop soma into a single point sphere soma \n",
        "\t\t *", simplify_section_extreme_str, "*:\t simplify morphology "
                                               "sectiones to the extreme with 2 "
                                               "points per section \n",
        "\n", "\n", "\tNote: Most operations are NOT commutative"
                    "\n");
    exit(1);
}

// parse transformation operations
morpho_operation_chain parse_transform_option(po::variables_map& options) {
    morpho_operation_chain filters;

    if (options.count("transform") < 1) {
        return filters;
    }

    const std::string string_option = options["transform"].as<std::string>();
    std::vector<std::string> vals =
        hadoken::string::tokenize(string_option, ",");

    if (vals.size() == 0) {
        transform_show_help();
    }

    for (const auto& operation : vals) {
        if (operation == delete_duplicate_point_operation_str) {
            filters.push_back(
                std::make_shared<delete_duplicate_point_operation>());

        } else if (operation == duplicate_first_point_operation_str) {
            filters.push_back(
                std::make_shared<duplicate_first_point_operation>());

        } else if (operation == soma_sphere_operation_str) {
            filters.push_back(std::make_shared<soma_sphere_operation>());

        } else if (operation == simplify_section_extreme_str) {
            filters.push_back(
                std::make_shared<simplify_section_extreme_operation>());
        } else {
            transform_show_help();
        }
    }
    return filters;
}

void transform_ops_print(const morpho_operation_chain& ops) {
    std::string res;
    if (ops.size() > 0) {
        fmt::scat(std::cout, "\napply the the filters: [ ");
        for (const auto& op : ops) {
            fmt::scat(std::cout, op->name(), " ");
        }

        fmt::scat(std::cout, "]\n");
    }

    fmt::scat(std::cout, "\n");
}

void export_morpho_to_gmsh(const std::string& filename_morpho,
                           const std::string& filename_geo,
                           po::variables_map& options) {

    std::vector<morpho_tree> trees;
    gmsh_exporter::exporter_flags flags = 0;
    if (options.count("single-soma")) {
        flags |= gmsh_exporter::exporter_single_soma;
    }

    if (options.count("with-dmg")) {
        flags |= gmsh_exporter::exporter_write_dmg;
    }

    if (options.count("with-bounding-box")) {
        flags |= gmsh_exporter::exporter_bounding_box;
    }

    if (options.count("dont-pack") == 0) {
        flags |= gmsh_exporter::exporter_packed;
    }

    morpho_operation_chain transform_ops = parse_transform_option(options);
    transform_ops_print(transform_ops);

    if (options.count("morpho-dir")) {
        const std::string morpho_dir = options["morpho-dir"].as<std::string>();
        const std::string circuit_filename = filename_morpho;

        fmt::scat(std::cout, "load circuit ", circuit_filename,
                  " with morphology directory ", morpho_dir, "\n");
        circuit::circuit_reader reader(circuit_filename, morpho_dir);

        trees = reader.create_all_morpho_tree();

    } else {
        fmt::scat(std::cout, "load one morphology into morphology tree ",
                  filename_morpho, "\n");

        {
            morpho_tree tree =
                morpho::reader::create_morpho_tree(filename_morpho);
            trees.emplace_back(std::move(tree));
        }
    }

    // apply all the specified transform ops

    std::for_each(trees.begin(), trees.end(), [&](morpho_tree& tree) {
        // apply user filters
        morpho_tree user_filtered_tree = morpho_transform(tree, transform_ops);

        // apply gmsh required filtered
        morpho_tree final_tree = morpho_transform(
            user_filtered_tree,
            {
                std::make_shared<delete_duplicate_point_operation>(),
                std::make_shared<duplicate_first_point_operation>(),
            });

        // we swap back the tree inside the vector
        tree.swap(final_tree);
    });

    gmsh_exporter exporter(std::move(trees), filename_geo, flags);
    exporter.set_identifier(std::string("morphology: ") + filename_morpho);

    if (options.count("point-cloud")) {
        exporter.export_to_point_cloud();

    } else if (options.count("3d-object")) {
        exporter.export_to_3d_object();
    } else {
        exporter.export_to_wireframe();
    }
    fmt::scat(std::cout, "\nconvert ", filename_morpho,
              " to gmsh file format.... ", filename_geo, "\n\n");
}

void export_morpho_to_x3d(const std::string& filename_morpho,
                          const std::string& filename_x3d,
                          po::variables_map& options) {
    (void)options;
    std::vector<morpho_tree> trees;

    fmt::scat(std::cout, "load morphology tree ", filename_morpho, "\n");

    auto tree_shared = load_morphology(filename_morpho);
    trees.emplace_back(std::move(*tree_shared));

    x3d_exporter exporter(std::move(trees), filename_x3d);

    exporter.export_to_sphere();

    fmt::scat(std::cout, "\nconvert ", filename_morpho,
              " to x3d file format.... ", filename_x3d, "\n\n");
}

void export_morpho_to_h5v1(const std::string& filename_morpho,
                           const std::string& filename_h5v1_output,
                           po::variables_map& options) {

    fmt::scat(std::cout, "load morphology tree ", filename_morpho, "\n");

    auto tree_sptr = load_morphology(filename_morpho);

    morpho_operation_chain transform_ops = parse_transform_option(options);
    transform_ops_print(transform_ops);

    auto transformed_tree = morpho_transform(*tree_sptr, transform_ops);

    h5_v1::morpho_writer writer(filename_h5v1_output);

    fmt::scat(std::cout, "\nconvert ", filename_morpho,
              " to h5v1 file format.... ", filename_h5v1_output, "\n\n");

    writer.write(transformed_tree);
}

std::string bool_to_string(bool v) { return (v) ? "true" : "false"; }

void print_morpho_stats(const std::string& morpho_file) {
    std::shared_ptr<morpho_tree> tree = load_morphology(morpho_file);
    fmt::scat(std::cout, "\n");
    fmt::scat(std::cout, "filename =  \"", morpho_file, "\"", "\n");
    fmt::scat(std::cout, "morphology_type = [\"detailed\", \"cones\" ]", "\n");
    fmt::scat(std::cout, "number_of_section = ",
              stats::total_number_sectiones(*tree), "\n");
    fmt::scat(std::cout, "number_of_points = ",
              stats::total_number_point(*tree), "\n");
    fmt::scat(std::cout, "min_radius_segment = ",
              stats::min_radius_segment(*tree), "\n");
    fmt::scat(std::cout, "max_radius_segment = ",
              stats::max_radius_segment(*tree), "\n");
    fmt::scat(std::cout, "median_radius_segment = ",
              stats::median_radius_segment(*tree), "\n");
    fmt::scat(std::cout, "has_duplicated_points = ",
              bool_to_string(stats::has_duplicated_points(*tree)), "\n");
    fmt::scat(std::cout, "\n");
}

int main(int argc, char** argv) {
    po::variables_map options;
    std::string help_string;
    try {
        auto parsed_options = parse_args(argc, argv, options, help_string);

        if (options.count("version")) {
            fmt::scat(std::cout, "version: ", version(), "\n");
            exit(0);
        }

        if (options.count("command") && options.count("subargs")) {
            std::string command = options["command"].as<std::string>();
            std::vector<std::string> subargs = po::collect_unrecognized(
                parsed_options.options, po::include_positional);
            if (command == "export") {

                if (subargs.size() == 4 && subargs[1] == "gmsh") {
                    export_morpho_to_gmsh(subargs[2], subargs[3], options);
                    return 0;
                }

                if (subargs.size() == 4 && subargs[1] == "x3d") {
                    export_morpho_to_x3d(subargs[2], subargs[3], options);
                    return 0;
                }

                if (subargs.size() == 4 && subargs[1] == "h5v1") {
                    export_morpho_to_h5v1(subargs[2], subargs[3], options);
                    return 0;
                }
            } else if (command == "stats") {
                if (subargs.size() == 2) {
                    print_morpho_stats(subargs[1]);
                    return 0;
                }
            }
        }

        fmt::scat(std::cout,
                  "\nWrong command usage, see --help for details\n\n");
    } catch (std::exception& e) {
        fmt::scat(std::cerr, argv[0], "\n", "Error ", e.what(), "\n");
        exit(-1);
    }
    return 0;
}
