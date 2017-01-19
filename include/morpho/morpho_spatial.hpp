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
#ifndef MORPHO_SPATIAL_HPP
#define MORPHO_SPATIAL_HPP

#include <memory>


#include <hadoken/geometry/geometry.hpp>


#include <morpho/morpho_tree.hpp>


namespace morpho{

class spatial_index_impl;

///
/// \brief spatial indexer for morphologies
///
/// Able to load any morphology and to check if a given point is part of a morphology or not
///
class spatial_index
{

public:
    spatial_index();
    ~spatial_index();

    ///
    /// \brief include a morpho tree and all its elements to the current spatial index
    /// \param tree
    ///
    void add_morpho_tree(const std::shared_ptr<morpho_tree> & tree);


    ///
    /// test if a point p is within a morphology space or not
    ///
    /// \return true if p is part of a morphology space else false
    ///
    bool is_within(const point & p) const;

private:

    spatial_index(const spatial_index &) = delete;
    spatial_index & operator=(const spatial_index &) = delete;

    std::unique_ptr<spatial_index_impl> _pimpl;
};


} // morpho

#endif // MORPHO_SPATIAL_HPP
