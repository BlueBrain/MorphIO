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


#include <morpho/morpho_stats.hpp>



namespace morpho{


namespace stats {


std::size_t total_number_branches(const morpho_tree &tree){
    return tree.get_tree_size();
}

std::size_t total_number_point(const morpho_tree &tree){

    std::vector<morpho_node const *> all_nodes = tree.get_all_nodes();
    std::size_t res = 0;

    std::for_each(all_nodes.begin(), all_nodes.end(), [&](morpho_node const* node_ref){

        if(node_ref->is_of_type(morpho_node_type::neuron_soma_type)){
            const neuron_soma & soma = static_cast<const neuron_soma&>(*node_ref);
            res += soma.get_line_loop().size();
        }else if(node_ref->is_of_type(morpho_node_type::neuron_branch_type)){
            const neuron_branch & branch = static_cast<const neuron_branch &>(*node_ref);
            res += branch.get_number_points();
        }

    });
    return res;
}


template<typename Fun, typename T>
T for_all_radius_reduction(const morpho_tree &tree, const Fun & f, T val){

    const T max_radius = val;
    double res = max_radius;

    std::vector<morpho_node const *> all_nodes = tree.get_all_nodes();

    std::for_each(all_nodes.begin(), all_nodes.end(), [&](morpho_node const* node){


        if(node->is_of_type(morpho_node_type::neuron_branch_type)){
            const neuron_branch & branch = static_cast<const neuron_branch &>(*node);
            const std::vector<double> & all_radius = branch.get_radius();

            res = std::accumulate(all_radius.begin(), all_radius.end(), res, [&](double a, double b){
                return f(a, b);
            });

        }

    });
    return res;
}

double min_radius_segment(const morpho_tree &tree){
    return for_all_radius_reduction(tree, [](double a, double b){
        return std::min(a, b);
    }, std::numeric_limits<double>::max());
}

double max_radius_segment(const morpho_tree &tree){
    return for_all_radius_reduction(tree, [](double a, double b){
        return std::max(a, b);
    }, std::numeric_limits<double>::min());
}


double median_radius_segment(const morpho_tree &tree){
    std::vector<morpho_node const *> all_nodes = tree.get_all_nodes();

    std::vector<double> all_radius;
    all_radius.reserve(total_number_point(tree));

    std::for_each(all_nodes.begin(), all_nodes.end(), [&](morpho_node const* node){

        if(node->is_of_type(morpho_node_type::neuron_branch_type)){
            const neuron_branch & branch = static_cast<const neuron_branch &>(*node);
            const std::vector<double> & all_radius_branch = branch.get_radius();
            std::copy(all_radius_branch.begin(), all_radius_branch.end(), std::back_inserter(all_radius));
        }

    });

    if(all_radius.size() < 1 ){
        throw std::invalid_argument("morpho_tree should contains at least one point");
    }

    std::sort(all_radius.begin(), all_radius.end());
    return all_radius[all_radius.size()/2];
}

} // stats


} //morpho
