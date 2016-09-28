#ifndef MORPHO_H5_V1_TPP
#define MORPHO_H5_V1_TPP


#include "../morpho_h5_v1.hpp"



#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <highfive/H5Object.hpp>
#include <highfive/H5DataSet.hpp>

#include <hadoken/format/format.hpp>

namespace morpho{

namespace h5_v1{

namespace {

inline void split_xyz_and_distance(const morpho_reader::mat_points & raw_points, branch::mat_points & points, branch::vec_double & distance){
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
            return soma_type;
        case 2:
            return axon_type;
        case 3:
            return dentrite_basal_type;
        case 4:
            return dentrite_apical_type;
         default:
            throw std::runtime_error("invalid cell type in morphology");
    }
}

}


namespace fmt = hadoken::format;

morpho_reader::morpho_reader(const std::__cxx11::string & myfilename)  :
    h5_file(myfilename),
    filename(myfilename),
    structures(h5_file.getDataSet("/structure")),
    points(h5_file.getDataSet("/points")){

}

morpho_reader::mat_points morpho_reader::get_points_raw() const{

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


morpho_reader::mat_points morpho_reader::get_soma_points_raw() const {



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



morpho_tree morpho_reader::create_morpho_tree() const{


    morpho_tree res;


    {
        // create soma
        std::unique_ptr<branch> soma(new branch(soma_type));

        mat_points raw_soma_points  =  get_soma_points_raw();
        branch::mat_points soma_points;
        branch::vec_double soma_distance;

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
            branch::mat_points branch_points;
            branch::vec_double branch_distance;

            auto raw_mat_range = get_branch_range_raw(i);

            branch_points.resize(raw_mat_range.second, 3);
            branch_distance.resize(raw_mat_range.second);

            // first line -> end segment previous branch
            const int prev_id = struct_raw(i, 2);

            // add all the points from this branch
            for(std::size_t p = 0; p < raw_mat_range.second; ++p){
                for(std::size_t c =0; c < 3; ++c){
                    branch_points(p, c) = points_raw(raw_mat_range.first +p, c);
                }
                branch_distance(p) = points_raw(raw_mat_range.first +p, 3);
            }

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
