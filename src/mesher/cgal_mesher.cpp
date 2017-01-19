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

#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Complex_2_in_triangulation_3.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/Complex_2_in_triangulation_3_file_writer.h>


#include <morpho/morpho_mesher.hpp>
#include <morpho/morpho_spatial.hpp>


// global
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::FT FT;
typedef K::Point_3 Point;
typedef std::function<FT (const Point&)> Function;

// 3D
// Domain

typedef CGAL::Implicit_mesh_domain_3<Function,K> Mesh_domain;

typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;

typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;


// Criteria
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

// To avoid verbose function and named parameters call
using namespace CGAL::parameters;


// 2D
// default triangulation for Surface_mesher
typedef CGAL::Surface_mesh_default_triangulation_3 Tr_2D;
typedef Tr_2D::Geom_traits GT_2D;

// c2t3
typedef CGAL::Complex_2_in_triangulation_3<Tr_2D> C2t3_2D;

typedef CGAL::Implicit_surface_3<GT_2D, Function> Surface_3;


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
    _flags(),
    _output_mesh_file(output_mesh_file),
    _tree(tree){

}

void morpho_mesher::set_mesh_tag(mesh_tag tag, bool value){
    _flags[int(tag)] = value;
}


void morpho_mesher::execute_3d_meshing(){
    std::cout << "1- Start meshing 3D" << std::endl;
    std::cout << "-- optimisation of mesh " << ((_flags[mesh_optimisation])?("ENABLED"):("DISABLED")) << "\n";

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
                       K::Sphere_3(CGAL::ORIGIN, max_distance*max_distance), 1.0 /1000000);
    // Mesh criteria
    //Spherical_sizing_field size;
    Mesh_criteria criteria(facet_angle=30, facet_size=0.1*8, facet_distance=0.025*8,
                           cell_radius_edge_ratio=2, cell_size=0.3 /*0.1*8*/);

    // Mesh generation
    std::cout << "4- Start mesh generation " << std::endl;

   auto t1 = std::chrono::system_clock::now();
    C3t3 c3t3;
    if(_flags[mesh_optimisation]){
        c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);
    } else{
        c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, CGAL::parameters::no_perturb(), CGAL::parameters::no_exude());
    }

    auto t2 = std::chrono::system_clock::now();

    std::cout << "\n";
    std::cout << "5- End mesh generation in " << std::chrono::duration_cast<std::chrono::seconds>(t2 -t1).count() << "s" << "\n";
    std::cout << "6- Meshing domain scan done with " << stats.total_iterations << " points checks and "
              << double(stats.within_iterations)/stats.total_iterations <<  " of positive matches " << "\n";


    // Output
    {
        const std::string local_3dmesh_output_file =_output_mesh_file + ".3d.mesh";
        std::cout << "- output 3D mesh to " << local_3dmesh_output_file << std::endl;

        std::ofstream medit_file(local_3dmesh_output_file);
        c3t3.output_to_medit(medit_file);
    }

    {
        const std::string local_surface_mesh_output_file =_output_mesh_file + ".surface.off";
        std::cout << "- output surface mesh to " << local_surface_mesh_output_file << std::endl;

        std::ofstream off_file(local_surface_mesh_output_file);
        c3t3.output_boundary_to_off(off_file);
    }

    std::cout << "7- End meshing " << std::endl;
}


void morpho_mesher::execute_surface_meshing(){
    Tr_2D tr;            // 3D-Delaunay triangulation
    C2t3_2D c2t3 (tr);   // 2D-complex in 3D-Delaunay triangulation


    std::cout << "1- Start meshing 2D (surface)" << std::endl;

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

    // defining the surface
    Surface_3 surface(domain_distance,             // pointer to function
                      K::Sphere_3(CGAL::ORIGIN, max_distance*max_distance), 1.0/1000000); // bounding sphere
    // Note that "2." above is the *squared* radius of the bounding sphere!
    // defining meshing criteria
    CGAL::Surface_mesh_default_criteria_3<Tr_2D> criteria(30.,  // angular bound
                                                       0.1*2,  // radius bound
                                                       0.1*2); // distance bound
    // meshing surface
    auto t1 = std::chrono::system_clock::now();
    if(_flags[force_manifold]){
        CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Manifold_tag());
    }else {
        CGAL::make_surface_mesh(c2t3, surface, criteria, CGAL::Non_manifold_tag());
    }

    auto t2 = std::chrono::system_clock::now();

    std::cout << "\n";
    std::cout << "5- End mesh generation in " << std::chrono::duration_cast<std::chrono::seconds>(t2 -t1).count() << "s" << "\n";
    std::cout << "6- Meshing domain scan done with " << stats.total_iterations << " points checks and "
              << double(stats.within_iterations)/stats.total_iterations <<  " of positive matches " << "\n";

    {
        const std::string local_surface_mesh_output_file =_output_mesh_file + ".surface.off";
        std::cout << "- output surface mesh to " << local_surface_mesh_output_file << std::endl;

        std::ofstream out(local_surface_mesh_output_file);
        CGAL::output_surface_facets_to_off (out, c2t3);

    }

    std::cout << "7- End meshing " << std::endl;

}


void morpho_mesher::execute(){
    if(_flags[only_surface]){
        execute_surface_meshing();
    }else {
        execute_3d_meshing();
    }
}




} // morpho



