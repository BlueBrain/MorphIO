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
#ifndef MORPHO_TRANSFORM_HPP
#define MORPHO_TRANSFORM_HPP

#include <vector>
#include <morpho/morpho_tree.hpp>

namespace morpho{

/// \brief  generic interface for morphology transformation operations
class morpho_operation{
public:
    virtual ~morpho_operation(){};

    /// apply the given operation on a morphology
    virtual morpho_tree apply(morpho_tree && morphology) = 0;

    /// return the name id of the operation
    virtual std::string name() =0;
};

///
/// \brief chain of operaitons on morphology for \ref morpho_transform
///
class morpho_operation_chain{
public:
    morpho_operation_chain();

    void append(morpho_operation* new_operation);

    morpho_operation& get(std::size_t chain_elem) const;

    std::size_t size() const;


private:
    std::vector<std::unique_ptr<morpho_operation> > _chain;
};



morpho_tree morpho_transform(morpho_tree && morphology, morpho_operation_chain & ops);


} // morpho


#include "bits/morpho_transform_bits.hpp"

#endif // MORPHO_TRANSFORM_HPP
