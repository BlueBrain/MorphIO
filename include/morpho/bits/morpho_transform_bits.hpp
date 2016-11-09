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
#ifndef MORPHO_TRANSFORM_BITS_HPP
#define MORPHO_TRANSFORM_BITS_HPP

#include <morpho/morpho_transform.hpp>

namespace morpho{


morpho_operation_chain::morpho_operation_chain(){

}

void morpho_operation_chain::append(morpho_operation* new_operation){
    _chain.emplace_back(std::unique_ptr<morpho_operation>(new_operation));
}

morpho_operation& morpho_operation_chain::get(std::size_t id) const{
    if(id >= _chain.size()){
        throw std::out_of_range("out of the chain of operation range");
    }

    return *(_chain[id]);
}

std::size_t morpho_operation_chain::size() const{
    return _chain.size();
}


morpho_tree morpho_transform(morpho_tree && morphology, morpho_operation_chain & ops){

    morpho_tree res = std::move(morphology);
    for(std::size_t i =0; i < ops.size(); ++i){
        res = ops.get(i).apply(std::move(res));
    }
    return std::move(res);
}


} // morpho

#endif // MORPHO_TRANSFORM_HPP
