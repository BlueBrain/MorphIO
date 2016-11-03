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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <iostream>
#include <exception>
#include <fstream>
#include <string>
#include <vector>
#include <utility>

#include <boost/program_options.hpp>

#include <boost/geometry.hpp>

#include <hadoken/format/format.hpp>

#include <morpho/morpho_h5_v1.hpp>

#include "mesh_exporter.hpp"
#include "x3d_exporter.hpp"

using namespace std;
using namespace morpho;

namespace fmt = hadoken::format;
namespace po = boost::program_options;

std::string version(){
    return std::string( MORPHO_VERSION_MAJOR "." MORPHO_VERSION_MINOR );
}

po::parsed_options parse_args(int argc, char** argv,
                         po::variables_map & res,
                         std::string & help_msg
                             ){
    po::options_description general("Commands:\n"
                                    "\t\t\n"
                                    "  export gmsh [morphology-file] [geo-file]:\texport morphology file to .geo file format\n"
                                    "  export x3d [morphology-file] [x3d-file]:\texport morphology file to .x3d file format\n"
                                    "\n\n"
                                    "Options");
    general.add_options()
        ("help", "produce a help message")
        ("version", "output the version number")
        ("point-cloud", "gmsh: export to a point cloud")
        ("wireframe", "gmsh: export to a wired morphology (default)")
        ("3d-object", "gmsh: export to a 3D object model")
        ("with-dmg", "gmsh: export to a dmg file format as well")
        ("single-soma", "gmsh: represent soma as a single element, point or sphere")
        ("sphere", "x3d: export cloud of sphere (default)")
        ("command", po::value<std::string>(), "command to execute")
        ("subargs", po::value<std::vector<std::string> >(), "Arguments for command");
        ;

    po::positional_options_description pos;
    pos.add("command", 1)
       .add("subargs", -1)
            ;

    auto opts = po::command_line_parser(argc, argv )
            .options(general)
            .positional(pos)
            .allow_unregistered()
            .run();
    po::store(opts, res);
    if(res.count("help")){
        fmt::scat(std::cout, general, "\n");
        exit(0);
    }
    help_msg = fmt::scat(general);
    return opts;
}



void export_morpho_to_mesh(const std::string & filename_morpho, const std::string & filename_geo,
                          po::variables_map & options){

    gmsh_exporter::exporter_flags flags = 0;
    if(options.count("single-soma")){
        flags |= gmsh_exporter::exporter_single_soma;
    }

    if(options.count("with-dmg")){
        flags |= gmsh_exporter::exporter_write_dmg;
    }

    gmsh_exporter exporter(filename_morpho, filename_geo, flags);

    if(options.count("point-cloud")){
        exporter.export_to_point_cloud();
 
    }else if(options.count("3d-object")){
        exporter.export_to_3d_object();
    }else{
        exporter.export_to_wireframe();
    }
    fmt::scat(std::cout, "\nconvert ", filename_morpho, " to gmsh file format.... ", filename_geo, "\n\n");    
}

void export_morpho_to_x3d(const std::string & filename_morpho, const std::string & filename_x3d,
                          po::variables_map & options){
    x3d_exporter exporter(filename_morpho, filename_x3d);

    exporter.export_to_sphere();

    fmt::scat(std::cout, "\nconvert ", filename_morpho, " to x3d file format.... ", filename_x3d, "\n\n");
}




int main(int argc, char** argv){
    po::variables_map options;
    std::string help_string;
    try{
        auto parsed_options = parse_args(argc, argv, options, help_string);

        if(options.count("version")){
            fmt::scat(std::cout, "version: ", version(), "\n");
            exit(0);
        }

        if(options.count("command") && options.count("subargs") ){
            std::string command = options["command"].as<std::string>();
            std::vector<std::string> subargs = po::collect_unrecognized(parsed_options.options, po::include_positional);
            if(command == "export" ){

                if(subargs.size() == 4
                    && subargs[1] == "gmsh"){
                    export_morpho_to_mesh(subargs[2], subargs[3], options);
                    return 0;
                }

                if(subargs.size() == 4
                    && subargs[1] == "x3d"){
                    export_morpho_to_x3d(subargs[2], subargs[3], options);
                    return 0;
                }
            };
        }

        fmt::scat(std::cout, "\nWrong command usage, see --help for details\n\n");
    }catch(std::exception & e){
        fmt::scat(std::cerr,
                  argv[0], "\n",
                "Error ", e.what(), "\n");
        exit(-1);
    }
    return 0;
}

