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
 **/

#include <iostream>
#include <string>
#include <functional>
#include <chrono>

#include <hadoken/format/format.hpp>

// CGAL import

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Implicit_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>


#include "morpho_mesher.hpp"
#include "morpho_spatial.hpp"


// Domain
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::FT FT;
typedef K::Point_3 Point;
typedef std::function<FT (const Point&)> Function;
typedef CGAL::Implicit_mesh_domain_3<Function,K> Mesh_domain;

typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;

typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;


// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

// To avoid verbose function and named parameters call
using namespace CGAL::parameters;



namespace morpho{

struct scan_stats{
    inline scan_stats() : total_iterations(0), within_iterations(0) {}

    std::size_t total_iterations, within_iterations;
};


FT is_part_of_morphotree(const Point& p, const morpho_tree & tree, const spatial_index & index,
                         scan_stats & stats){
    point my_point(p.x(), p.y(), p.z());
    stats.total_iterations++;

   if(stats.total_iterations%100000 ==0){
        hadoken::format::scat(std::cout, "Geometry scan progress: ", stats.total_iterations, " points check with ",
                              stats.within_iterations," positives matches", "\r");
    }

    if(index.is_within(my_point)){
            stats.within_iterations++;
            return -1;
    }

    return 1;
}



morpho_mesher::morpho_mesher(const std::shared_ptr<morpho_tree> & tree, const std::string &output_mesh_file) :
    _output_mesh_file(output_mesh_file),
    _tree(tree){

}


void morpho_mesher::execute(){
    std::cout << "1- Start meshing" << std::endl;

    // get global bounding box
    box g_box = _tree->get_bounding_box();
    double max_distance = hg::distance(g_box.min_corner(), point(0,0,0));
    max_distance = std::max(max_distance, hg::distance(g_box.min_corner(), point(0,0,0)));

    std::cout << "2- Configure bounding sphere with radius of " << max_distance << std::endl;

    // spatial indexing
    std::cout << "3- Create spatial index " << std::endl;

    spatial_index morpho_indexer;
    morpho_indexer.add_morpho_tree(_tree);


    // Domain (Warning: Sphere_3 constructor uses squared radius !)
    scan_stats stats;
    Function domain_distance = [&](const Point& p){
        return is_part_of_morphotree(p, *_tree, morpho_indexer, stats);
    };

    Mesh_domain domain(domain_distance,
                       K::Sphere_3(CGAL::ORIGIN, max_distance*max_distance));
    // Mesh criteria
    Mesh_criteria criteria(facet_angle=30, facet_size=0.1*10, facet_distance=0.025*10,
                           cell_radius_edge_ratio=2, cell_size=0.1*10);

    // Mesh generation
    std::cout << "4- Start mesh generation " << std::endl;

    auto t1 = std::chrono::system_clock::now();
    C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);
    auto t2 = std::chrono::system_clock::now();

    std::cout << "\n";
    std::cout << "5- End mesh generation in " << std::chrono::duration_cast<std::chrono::seconds>(t2 -t1).count() << "s" << "\n";
    std::cout << "6- Meshing domain scan done with " << stats.total_iterations << " points checks and "
              << double(stats.within_iterations)/stats.total_iterations <<  " of positive matches " << "\n";


    // Output
    const std::string local_mesh_output_file =_output_mesh_file + ".mesh";
    std::cout << "output meshing to " << local_mesh_output_file << std::endl;

    std::ofstream medit_file(local_mesh_output_file);
    c3t3.output_to_medit(medit_file);

}


} // morpho



