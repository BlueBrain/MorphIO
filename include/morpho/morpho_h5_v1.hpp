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
#ifndef MORPHO_H5_V1_HPP
#define MORPHO_H5_V1_HPP

#ifndef H5_USE_BOOST
#define H5_USE_BOOST
#endif

#include <string>
#include <utility>
#include <unordered_map>


#include "morpho_types.hpp"

#include <highfive/H5File.hpp>

#include "morpho_tree.hpp"



namespace morpho{

namespace h5_v1{

using namespace boost::numeric;

class morpho_reader{
public:

    typedef std::pair<std::size_t, std::size_t> range;
    typedef std::unordered_map<std::string, std::string> meta_map;

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
    /// \brief hashmap of the properties (str->str)
    /// \return meta_t
    ///
    inline meta_map   get_metadata() const;
    template <typename T>
    inline T get_metadata(const std::string attr) const;
    inline cell_family get_cell_family() const;


    ///
    /// \brief get_filename
    /// \return filename of the associated morphology
    ///
    inline const std::string & get_filename() const{
        return filename;
    }


    inline morpho_tree create_morpho_tree() const;


private:
    HighFive::File h5_file;
    std::string filename;

    // datasets
    HighFive::DataSet structures, points;

    // Metadata group
    HighFive::Group metadata;
};


class morpho_writer{
public:
    // create a new h5v1 morphology file at filename path
    inline morpho_writer(const std::string & filename);

    // write a new morphology file out of the given morpho_tree
    inline void write(const morpho_tree & tree);

private:
    HighFive::File h5_file;
    std::string filename;

};


} //h5_v1



} // morpho


#include "bits/morpho_h5_v1.tpp"

#endif // MORPHO_H5_V1_HPP

