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
#include "x3d_exporter.hpp"

#include <boost/filesystem/path.hpp>
#include <hadoken/format/format.hpp>

#include <morpho/morpho_tree.hpp>

namespace fmt = hadoken::format;
namespace geo = hadoken::geometry::cartesian;
namespace fs = boost::filesystem;

namespace morpho {

void points_distance_to_sphere(const point& my_point, double distance,
                               const std::string& sphere_unit_name,
                               std::ostream& output) {

    fmt::scat(output, "<Transform translation='", geo::get_x(my_point), " ",
              geo::get_y(my_point), " ", geo::get_z(my_point), "' scale='",
              distance, " ", distance, " ", distance, "' >"
                                                      "\n"
                                                      "  <Shape USE=\"",
              sphere_unit_name, "\" />"
                                "\n"
                                "</Transform>"
                                "\n"

              );
}

x3d_exporter::x3d_exporter(std::vector<morpho_tree>&& trees,
                           const std::string& x3d_filename)
    : morphotrees(std::move(trees)), identifier_string(),
      x3d_stream(x3d_filename), dest_filename(x3d_filename) {}

void x3d_exporter::export_to_sphere() {
    envelop_header_and_footer([this] { this->export_all_points(); });
}

static std::string get_sphere(const std::string reference_name) {
    return fmt::scat(
        "<Shape DEF=\"", reference_name,
        "\">"
        "\n"
        "  <Sphere radius='1.0' /> "
        "\n"
        "  <Appearance>"
        "\n"
        "    <Material DEF='MaterialLightBlue' diffuseColor='0.1 0.5 1'/>"
        "\n"
        "  </Appearance>"
        "\n"
        "</Shape>        "
        "\n");
}

void x3d_exporter::export_all_points() {

    for (auto& tree : morphotrees) {
        fmt::scat(x3d_stream, "    <Group>"
                              "\n");

        const std::string sphere_unit_name = "baseSphere";

        // export base sphere
        fmt::scat(x3d_stream, get_sphere(sphere_unit_name));

        // export soma
        auto& soma = static_cast<const neuron_soma&>(tree.get_node(0));
        auto sphere = soma.get_sphere();
        points_distance_to_sphere(sphere.get_center(), sphere.get_radius(),
                                  sphere_unit_name, x3d_stream);

        // export points
        for (std::size_t b_id = 1; b_id < tree.get_tree_size(); ++b_id) {

            auto& section =
                static_cast<const neuron_section&>(tree.get_node(b_id));
            auto& points = section.get_points();
            auto& distance = section.get_radius();

            assert(points.size() == distance.size());
            for (std::size_t i = 0; i < points.size(); ++i) {
                points_distance_to_sphere(points[i], distance[i],
                                          sphere_unit_name, x3d_stream);
            }
        }

        fmt::scat(x3d_stream, "    </Group>"
                              "\n");
    }
}

void x3d_exporter::envelop_header_and_footer(
    const std::function<void()>& fcontent) {

    fmt::scat(x3d_stream,
              "<X3D profile='Immersive' version='3.3' "
              "xmlns:xsd='http://www.w3.org/2001/XMLSchema-instance' "
              "xsd:noNamespaceSchemaLocation='http://www.web3d.org/"
              "specifications/x3d-3.3.xsd'>"
              "\n"
              "  <head>"
              "\n"
              "  </head>"
              "\n"
              "  <Scene>"
              "\n"
              "<!-- Scene generated with morpho-tool from morphology",
              identifier_string, " -->"
                                 "\n"
                                 " <WorldInfo title='",
              identifier_string, "'/>"
                                 "\n");

    fcontent();
    fmt::scat(x3d_stream, "  </Scene>"
                          "\n"
                          "\n"
                          "</X3D>"
                          "\n");

    html_viewer();
}

void x3d_exporter::html_viewer() {
    fs::path html_filename = fs::path(dest_filename).parent_path();
    html_filename /= fs::path(dest_filename).stem();

    std::ofstream ohtml(fmt::scat(html_filename.native(), "_page.html"));

    fmt::scat(
        ohtml, "<html>"
               "\n"
               "<head>"
               "\n"
               "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>"
               "\n"
               "    <title>Rendering test page for ",
        dest_filename, " </title>"
                       "\n"
                       "    <script type='text/javascript' "
                       "src='http://www.x3dom.org/download/x3dom.js'> </script>"
                       "\n"
                       "    <link rel='stylesheet' type='text/css' "
                       "href='http://www.x3dom.org/download/x3dom.css'/>"
                       "\n"
                       "</head>"
                       "\n"
                       "<body>"
                       "\n"
                       "<h1>Rendering test page for ",
        dest_filename,
        "</h1>"
        "\n"
        "<script>"
        "\n"
        "</script>"
        "\n"
        "<x3d width='1200px' height='800px'>"
        "\n"
        "    <scene>"
        "\n"
        "            <Inline nameSpaceName=\"Morpho\" mapDEFToID=\"true\""
        "\n"
        "                     url=\"",
        dest_filename, "\" />"
                       "\n"
                       "    </scene>"
                       "\n"
                       "</x3d>"
                       "\n"
                       "</body>"
                       "\n"
                       "</html>");
}

} // morpho
