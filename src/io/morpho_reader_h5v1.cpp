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


#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_stats.hpp>

#include <tuple>
#include <chrono>

#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <highfive/H5Object.hpp>
#include <highfive/H5DataSet.hpp>

#include <hadoken/format/format.hpp>

#include <hadoken/geometry/geometry.hpp>
#include <hadoken/ublas/ublas.hpp>

#define _DEBUG true

namespace morpho{

namespace h5_v1{

namespace {

void split_xyz_and_distance(const mat_points & raw_points, std::vector<point> & points, std::vector<double> & radius){
    points.resize(raw_points.size1());
    radius.resize(raw_points.size1());

    for(auto i = std::size_t(0); i < raw_points.size1(); ++i ){
        points[i] = point( raw_points(i,0), raw_points(i,1), raw_points(i,2));
        radius[i] = raw_points(i,3) / 2.0 ; // radius -> /2
    }
}


neuron_struct_type branch_type_from_h5v1(const int type_id){
    switch(type_id){
        case 1:
            return neuron_struct_type::soma;
        case 2:
            return neuron_struct_type::axon;
        case 3:
            return neuron_struct_type::dentrite_basal;
        case 4:
            return neuron_struct_type::dentrite_apical;
         default:
            throw std::runtime_error("invalid cell type in morphology");
    }
}

int h5v1_from_branch_type(neuron_struct_type btype){
    switch(btype){
        case neuron_struct_type::soma:
            return 1;
        case neuron_struct_type::axon:
            return 2;
        case neuron_struct_type::dentrite_basal:
            return 3;
        case neuron_struct_type::dentrite_apical:
            return 4;
         default:
            throw std::runtime_error("invalid cell type in morphology");
    }
}

glia_struct_type glia_branch_type_from_h5v1( const int type_id ){
    switch(type_id) {
        case 1:
            return glia_struct_type::soma;
        case 2:
            return glia_struct_type::glia_process;
        case 3:
            return glia_struct_type::glia_endfoot;
        default:
            throw std::runtime_error("invalid glia cell type in morphology");
    }
}
}



namespace fmt = hadoken::format;

morpho_reader::morpho_reader(const std::string & myfilename)  :
    h5_file(myfilename),
    filename(myfilename),
    structures(h5_file.getDataSet("/structure")),
    points(h5_file.getDataSet("/points")) {

    if ( h5_file.exist("metadata") ) {
        metadata = h5_file.getGroup("/metadata");
    }
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


mat_index morpho_reader::get_struct_raw() const {
    mat_index res;
    structures.read<mat_index>(res);

    return res;
}


cell_family morpho_reader::get_cell_family() const  {

    if( ! metadata.hasAttribute("cell_family") ) {
        return cell_family::NEURON;
    }

    int cell_type = get_metadata<int>("cell_family");

    if (_DEBUG)
        std::cout << "Cell has specification of family: " << cell_type << std::endl;

    if (cell_type == 0) {
        return cell_family::NEURON;
    }
    if (cell_type == 1) {
        return cell_family::GLIA;
    }
    throw std::runtime_error("invalid cell family in morphology: " + cell_type);
}


morpho_reader::meta_map morpho_reader::get_metadata() const {
    morpho_reader::meta_map metaprops;

    std::vector<std::string> all_metas = metadata.listAttributeNames();
    std::string prop_value;

    for(std::string attr_name : all_metas) {
        HighFive::Attribute att = metadata.getAttribute(attr_name);
        att.read(prop_value);
        metaprops[attr_name] = prop_value;
    }

    return metaprops;
}

template<typename T>
T morpho_reader::get_metadata(const std::string attr_name) const {
    HighFive::Attribute att = metadata.getAttribute(attr_name);
    T prop_value;
    att.read(prop_value);
    return prop_value;
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

        std::shared_ptr<neuron_soma> soma;
        mat_points raw_soma_points  =  get_soma_points_raw();

        std::vector<point> soma_points;
        std::vector<double> soma_distance;

        split_xyz_and_distance(raw_soma_points, soma_points, soma_distance);

        // if single element
        //  -> we need to modelise the soma as a sphere : one point + radius
        // else
        //  -> we have a line loop, import all points
        if(soma_points.size() == 1 ){
            soma.reset(new neuron_soma(soma_points[0], soma_distance[0]));
        }else {
            soma.reset(new neuron_soma(std::move(soma_points)));
        }

        res.add_node(-1, std::static_pointer_cast<morpho_node>(soma));
    }

    {
        namespace ublas = boost::numeric::ublas;

        auto points_raw = get_points_raw();
        auto struct_raw = get_struct_raw();

        // create branch
        const std::size_t n_branch = structures.getSpace().getDimensions()[0];
        for(std::size_t i = 1; i < n_branch; ++i){
            mat_points branch_raw;

            auto raw_mat_range = get_branch_range_raw(i);

            const std::size_t n_row = raw_mat_range.second;
            const std::size_t n_col = 4;
            branch_raw.resize(n_row, n_col);

            // first line -> end segment previous branch
            const int prev_id = struct_raw(i, 2);

            mat_range_points range_points_raw(points_raw, morpho::range(raw_mat_range.first, raw_mat_range.first+ n_row), morpho::range(0, n_col));
            branch_raw = range_points_raw;

            std::vector<point> branch_points;
            std::vector<double> branch_radius;

            split_xyz_and_distance(range_points_raw, branch_points, branch_radius);


            std::shared_ptr<neuron_branch> b(new neuron_branch(
                                                 branch_type_from_h5v1(struct_raw(i, 1)),
                                                 std::move(branch_points),
                                                 std::move(branch_radius))
                                            );
            res.add_node(prev_id, b);

        }
    }


    return res;
}



morpho_writer::morpho_writer(const std::string & f)  :
    h5_file(f, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate),
    filename(f)
{

}


static void export_tree_to_raw(const morpho_tree & tree, mat_index & raw_index, mat_points & raw_points){
    const std::size_t number_node = tree.get_tree_size();
    std::size_t offset_struct = 0;
    std::size_t offset_points = 0;

    for(std::size_t i = 0 ; i < number_node; ++i){
        const morpho_node & node = tree.get_node(i);

        if(node.is_of_type(morpho_node_type::neuron_soma_type)){
            const neuron_soma & soma = static_cast<const neuron_soma&>(node);
            auto line_loop = soma.get_line_loop();
            raw_index(offset_struct, 0) = offset_points;
            raw_index(offset_struct, 1) = h5v1_from_branch_type(neuron_struct_type::soma); // soma
            raw_index(offset_struct, 2) = -1;
            offset_struct++;

            if(line_loop.size() != 1 ){ // real line loop

                for(const auto & point : line_loop){
                    raw_points(offset_points, 0) = point(0);
                    raw_points(offset_points, 1) = point(1);
                    raw_points(offset_points, 2) = point(2);
                    raw_points(offset_points, 3) = 0.0; // arbitrary radius for soma ring
                    offset_points ++;
                }
            }else{ // simple soma sphere
                // single point with propre radius for soma sphere
                sphere soma_sphere = soma.get_sphere();
                raw_points(offset_points, 0) = soma_sphere.get_center()(0);
                raw_points(offset_points, 1) = soma_sphere.get_center()(1);
                raw_points(offset_points, 2) = soma_sphere.get_center()(2);
                raw_points(offset_points, 3) = soma_sphere.get_radius() *2.0;
                offset_points ++;
            }
        }else if(node.is_of_type(morpho_node_type::neuron_branch_type)){
            const neuron_branch & branch = static_cast<const neuron_branch&>(node);
            const auto & points = branch.get_points();
            const auto & radius = branch.get_radius();
            assert(points.size() == radius.size());

            raw_index(offset_struct, 0) = offset_points;
            raw_index(offset_struct, 1) = h5v1_from_branch_type(branch.get_branch_type()); // soma
            raw_index(offset_struct, 2) = tree.get_parent(i);
            offset_struct++;

            std::size_t index = 0;
            for(const auto & point : points){
                raw_points(offset_points, 0) = point(0);
                raw_points(offset_points, 1) = point(1);
                raw_points(offset_points, 2) = point(2);
                raw_points(offset_points, 3) = radius[index] * 2.0 ; // h5v1 store diameter
                index ++;
                offset_points ++;
            }
        }
    }

}

void morpho_writer::write(const morpho_tree &tree){
    using namespace HighFive;
    const std::size_t number_of_branch = stats::total_number_branches(tree);
    const std::size_t number_of_points = stats::total_number_point(tree);

    std::time_t time_point = std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point());

    mat_points raw_points(number_of_points, 4);
    mat_index raw_struct(number_of_branch, 3);

    std::vector<std::string> comment = { fmt::scat(" created out by morpho_tool v1",
                                            " the ", std::ctime(&time_point)) };

    DataSet dpoints = h5_file.createDataSet<double>("/points", DataSpace::From(raw_points));
    DataSet dstructures = h5_file.createDataSet<int32_t>("/structure",DataSpace::From(raw_struct));
    Attribute acomment = h5_file.createAttribute<std::string>("comment", DataSpace::From(comment));


    export_tree_to_raw(tree, raw_struct, raw_points);


    dpoints.write(raw_points);
    dstructures.write(raw_struct);
    // add a comment to trace generation
    acomment.write(comment);
}


} //h5_v1

} // morpho


