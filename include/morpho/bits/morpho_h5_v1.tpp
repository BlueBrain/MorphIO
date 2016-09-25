#ifndef MORPHO_H5_V1_TPP
#define MORPHO_H5_V1_TPP

#include "../morpho_h5_v1.hpp"

#include <highfive/H5Object.hpp>
#include <highfive/H5DataSet.hpp>

namespace morpho{

namespace h5_v1{


morpho_reader::morpho_reader(const std::__cxx11::string &filename)  : h5_file(filename){

}

morpho_reader::array_points morpho_reader::get_points() const{
    array_points res;
    auto dataset = h5_file.getDataSet("/points");

    dataset.read<array_points>(res);

    return res;
}




} //h5_v1



} // morpho


#endif // MORPHO_H5_V1_TPP
