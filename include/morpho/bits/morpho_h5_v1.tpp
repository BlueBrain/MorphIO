#ifndef MORPHO_H5_V1_TPP
#define MORPHO_H5_V1_TPP


#include "../morpho_h5_v1.hpp"



#include <boost/numeric/ublas/blas.hpp>

#include <highfive/H5Object.hpp>
#include <highfive/H5DataSet.hpp>

#include <hadoken/format/format.hpp>

namespace morpho{

namespace h5_v1{

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

    const std::size_t nelem_branch = ((structure_ids.size1() == 2) ? structure_ids(1, 0) : n_struct) - offset_branch;

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




} //h5_v1



} // morpho


#endif // MORPHO_H5_V1_TPP
