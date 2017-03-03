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
#ifndef MORPHO_H5_V1_TPP
#define MORPHO_H5_V1_TPP


#include "../morpho_h5_v1.hpp"

#include <tuple>

#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <highfive/H5Object.hpp>
#include <highfive/H5DataSet.hpp>

#include <hadoken/format/format.hpp>

#include <hadoken/geometry/geometry.hpp>
#include <hadoken/ublas/ublas.hpp>

namespace morpho{

namespace h5_v1{

namespace {

inline void split_xyz_and_distance(const mat_points & raw_points, mat_points & points, vec_double & distance){
    points.resize(raw_points.size1(), 3);
    distance.resize(raw_points.size1());

    for(auto i = std::size_t(0); i < raw_points.size1(); ++i ){
        points(i,0) = raw_points(i,0);
        points(i,1) = raw_points(i,1);
        points(i,2) = raw_points(i,2);
        distance(i) = raw_points(i,3);
    }
}


inline branch_type branch_type_from_h5v1(const int type_id){
    switch(type_id){
        case 1:
            return branch_type::soma;
        case 2:
            return branch_type::axon;
        case 3:
            return branch_type::dentrite_basal;
        case 4:
            return branch_type::dentrite_apical;
         default:
            throw std::runtime_error("invalid cell type in morphology");
    }
}

}


namespace fmt = hadoken::format;

morpho_reader::morpho_reader(const std::string & myfilename)  :
    h5_file(myfilename),
    filename(myfilename),
    structures(h5_file.getDataSet("/structure")),
    points(h5_file.getDataSet("/points")){

}

mat_points morpho_reader::get_points_raw() const{

    mat_points res;

    points.read<mat_points>(res);

    return res;
}


morpho_reader::range morpho_reader::get_branch_range_raw(int id) const{
    mat_index structure_ids;

    const std::size_t n_struct = structures.getSpace().getDimensions()[0];
    const std::size_t n_points = points.getSpace().getDimensions()[0];

    if(id < 0 || std::size_t(id) >= n_struct){
        throw std::runtime_error(fmt::scat(" branch ", id, " out of range"));
    }

    const bool is_last_branch = std::size_t(id) >= (n_struct-1);
    std::size_t nlines = (is_last_branch) ? 1: 2;

    auto slice = structures.select({std::size_t(id), std::size_t(0)},
                                    { nlines, std::size_t(3)});

    slice.read<mat_index>(structure_ids);


    assert(structure_ids.size2() == 3);
    assert(structure_ids.size1() == 1 || structure_ids.size1() == 2);

    const std::size_t offset_branch = structure_ids(0, 0);

    const std::size_t nelem_branch = ((structure_ids.size1() == 2) ? structure_ids(1, 0) : n_points) - offset_branch;

    if( offset_branch >= n_points || nelem_branch >= n_points){
        throw std::runtime_error(fmt::scat("invalid offset/counter in morphology at branch ", id));
    }

    return std::make_pair(offset_branch, nelem_branch);
}


mat_points morpho_reader::get_soma_points_raw() const {



    mat_points res;
    mat_index structure_soma;

    {

        auto slice = structures.select({0, 0}, {1, 3});
        slice.read<mat_index>(structure_soma);

        if(structure_soma(0, 1) != 1){
            throw std::runtime_error(fmt::scat("morphology ", filename,
                                     " does not have a soma for first /structure element "));
        }
    }

    auto range_soma = get_branch_range_raw(0);

    auto slice_soma = points.select({range_soma.first, 0}, {range_soma.second, 4});

    slice_soma.read(res);

    return res;
}


morpho_reader::mat_index morpho_reader::get_struct_raw() const {
    mat_index res;

    structures.read<mat_index>(res);

    return res;
}

/// check if branch has duplicted points with parent
static inline bool check_duplicated_point(mat_range_points & prev_range, mat_range_points & range){
    namespace geo = hadoken::geometry::cartesian;
    std::size_t prev_range_last_elem = prev_range.size1()-1;
    const geo::point3d point_prev(prev_range(prev_range_last_elem, 0), prev_range(prev_range_last_elem, 1), prev_range(prev_range_last_elem, 2));
    const geo::point3d point_new(range(0, 0), range(0, 1), range(0, 2));

    if( geo::distance(
                point_prev,
                point_new) <= 0.001){
        std::cout << "duplicated point" << std::endl;
        return true;
    }
    std::cout << "not duplicated" << std::endl;
    return false;
 }


morpho_tree morpho_reader::create_morpho_tree() const{

    namespace ublas = boost::numeric::ublas;

    morpho_tree res;


    {
        // create soma
        std::unique_ptr<branch> soma(new branch_soma());

        mat_points raw_soma_points  =  get_soma_points_raw();

        mat_points soma_points;
        vec_double soma_distance;

        split_xyz_and_distance(raw_soma_points, soma_points, soma_distance);

        soma->set_points(std::move(soma_points), std::move(soma_distance));
        res.set_root(std::move(soma));
    }

    {
        namespace ublas = boost::numeric::ublas;

        auto points_raw = get_points_raw();
        auto struct_raw = get_struct_raw();

        // create branch
        const std::size_t n_branch = structures.getSpace().getDimensions()[0];
        for(std::size_t i = 1; i < n_branch; ++i){
            mat_points branch_points;
            vec_double branch_distance;

            auto raw_mat_range = get_branch_range_raw(i);

            const std::size_t n_row = raw_mat_range.second;
            const std::size_t n_col = 3;
            branch_points.resize(n_row, n_col);
            branch_distance.resize(n_row);

            // first line -> end segment previous branch
            const int prev_id = struct_raw(i, 2);

            mat_range_points range_points_raw(points_raw, morpho::range(raw_mat_range.first, raw_mat_range.first+ n_row), morpho::range(0, n_col));
            branch_points = range_points_raw;
            branch_distance = ublas::subrange( ublas::column(points_raw, 3), raw_mat_range.first, raw_mat_range.first+ n_row);

            // convert diameter into radius
            std::for_each(branch_distance.begin(), branch_distance.end(), [](vec_double::value_type & elem){
               elem /= 2.0;
            });

            std::unique_ptr<branch> b(new branch(branch_type_from_h5v1(struct_raw(i, 1))));
            b->set_points(std::move(branch_points), std::move(branch_distance));

            res.add_child(prev_id, std::move(b));

        }
    }


    return res;
}






} //h5_v1



} // morpho


#endif // MORPHO_H5_V1_TPP
