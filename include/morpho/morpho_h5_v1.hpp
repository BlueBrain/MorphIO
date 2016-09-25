#ifndef MORPHO_H5_V1_HPP
#define MORPHO_H5_V1_HPP

#include <string>
#include <boost/multi_array.hpp>

#include <highfive/H5File.hpp>


namespace morpho{

namespace h5_v1{

class morpho_reader{
public:
    typedef boost::multi_array<double, 2> array_points;

    morpho_reader(const std::string & filename);

    array_points get_points() const;


private:
    HighFive::File h5_file;
};



} //h5_v1



} // morpho


#include "bits/morpho_h5_v1.tpp"

#endif // MORPHO_H5_V1_HPP

