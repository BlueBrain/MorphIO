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

#include <boost/geometry.hpp>

#include <hadoken/format/format.hpp>

#include <morpho/morpho_h5_v1.hpp>

#include "mesh_exporter.hpp"

using namespace std;
using namespace morpho;

namespace fmt = hadoken::format;

std::string version(){
    return std::string( MORPHO_VERSION_MAJOR "." MORPHO_VERSION_MINOR );
}




void export_morpho_to_mesh(const std::string & filename_morpho, const std::string & filename_geo){
    gmsh_exporter exporter(filename_morpho, filename_geo);

    exporter.export_to_point_cloud();
}


int main(int argc, char** argv){

    try{
        export_morpho_to_mesh(argv[1], argv[2]);
    }catch(std::exception & e){
        std::cerr <<  argv[0] << " Error " << e.what() << std::endl;
        exit(-1);
    }
    return 0;
}

