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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
*/

#include <morpho/morpho_tree_algorithm.hpp>

namespace morpho {

// find node of a given neuron 3d type
std::vector<int> find_neuron_nodes(const morpho_tree& tree,
                                   neuron_struct_type mtype) {
    std::vector<int> res;

    res.reserve(tree.get_tree_size());

    int cur_idx = 0;
    for (auto& node : tree.get_all_nodes()) {
        if (node->is_of_type(morpho_node_type::neuron_node_3d_type)) {
            neuron_node_3d const* nodex =
                static_cast<neuron_node_3d const*>(node);
            if (nodex->get_section_type() == mtype) {
                res.push_back(cur_idx);
            }
        }
        cur_idx++;
    }
    return res;
}

// find the neuron soma
neuron_soma const* find_neuron_soma(const morpho_tree& tree) {

    for (auto& node : tree.get_all_nodes()) {
        if (node->is_of_type(morpho_node_type::neuron_soma_type)) {
            return static_cast<neuron_soma const*>(node);
        }
    }

    return nullptr;
}

} // morpho
