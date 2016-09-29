#ifndef MORPHO_H5_V1_HPP
#define MORPHO_H5_V1_HPP

#include <string>
#include <utility>

#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/matrix.hpp>


#include <highfive/H5File.hpp>

#include "morpho_tree.hpp"


namespace morpho{


namespace h5_v1{

using namespace boost::numeric;

class morpho_reader{
public:
    enum generate_flags{
        generate_default = 0x00,
        generate_single_soma = 0x01
    };

    typedef ublas::matrix<double> mat_points;
    typedef ublas::matrix<int> mat_index;

    typedef std::pair<std::size_t, std::size_t> range;

    inline morpho_reader(const std::string & filename);

    ///
    /// \brief get all points associated with the morphology
    /// \return all the points of the morphology
    ///  in a matrix(N, 4) with (x,y,z, d)
    ///
    ///  x : position on axis X
    ///  y: position on axis Y
    ///  z: position on axis Z
    ///  d: diameter of the segment associated with the points
    ///
    ///
    inline mat_points get_points_raw() const;


    ///
    /// \brief get all points associated with the soma
    /// \return all the points of the morphology
    ///  in a matrix(N, 4) with (x,y,z, d)
    ///
    ///  x: position on axis X
    ///  y: position on axis Y
    ///  z: position on axis Z
    ///  d: not relevant
    ///
    inline mat_points get_soma_points_raw() const;


    inline mat_index get_struct_raw() const;

    ///
    /// \brief points range of a given branch by id
    /// \return range( offset, count )
    ///
    inline range get_branch_range_raw(int id ) const;


    ///
    /// \brief get_filename
    /// \return filename of the associated morphology
    ///
    inline const std::string & get_filename() const{
        return filename;
    }


    inline morpho_tree create_morpho_tree(generate_flags flags = generate_default) const;


private:
    HighFive::File h5_file;
    std::string filename;

    // datasets
    HighFive::DataSet structures, points;
};



} //h5_v1



} // morpho


#include "bits/morpho_h5_v1.tpp"

#endif // MORPHO_H5_V1_HPP

