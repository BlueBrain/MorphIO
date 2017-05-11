/*
 * Copyright (C) 2017 Adrien Devresse <adrien.devresse@epfl.ch>
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
#ifndef MORPHO_TREE_ALGORITHM_HPP
#define MORPHO_TREE_ALGORITHM_HPP

#include <morpho/morpho_tree.hpp>


namespace morpho {

/// return IDs for neuron struct type
std::vector<int> find_neuron_nodes(const morpho_tree & tree, neuron_struct_type mtype);

/// return the first encountered soma in the tree
neuron_soma const* find_neuron_soma(const morpho_tree & tree);


} // morpho

#endif // MORPHO_TREE_ALGORITHM_HPP
