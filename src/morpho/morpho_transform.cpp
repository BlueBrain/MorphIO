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


#include <morpho/morpho_transform.hpp>
#include <morpho/morpho_transform_filters.hpp>






namespace morpho{

//
// transform operations
//
morpho_tree morpho_transform(const morpho_tree & tree, morpho_operation_chain & ops){
    morpho_tree res = tree;

    for(std::shared_ptr<morpho_operation> & op : ops){
        res = op->apply(res);
    }
    return res;
}


//
// base morpho operation
//

morpho_operation::morpho_operation(){

}


morpho_operation::~morpho_operation(){

}

//
// delete duplicated points
//

delete_duplicate_point_operation::delete_duplicate_point_operation(){

}

// return last point of the parent branch, or empty optional if not possible (e.g root branch, no points in parent )
boost::optional<point> last_point_from_parent(const morpho_tree & tree, int node_id){
    int parent_id = tree.get_parent(node_id);

    // first node, error
    if(parent_id < 0)    {
        return boost::optional<point>();
    }
    const morpho_node & parent_node = tree.get_node(parent_id);

    if(parent_node.is_of_type(morpho_node_type::neuron_branch_type)){
        const neuron_branch & b = static_cast<const neuron_branch&>(parent_node);
        const std::size_t branch_size = b.get_points().size();
        if(branch_size >= 1){
            return boost::optional<point>(b.get_points()[branch_size-1]);
        }
    }else if(parent_node.is_of_type(morpho_node_type::neuron_soma_type)){
        const neuron_soma & soma = static_cast<const neuron_soma&>(parent_node);
        return boost::optional<point>(soma.get_sphere().get_center());
    }
    return boost::optional<point>();
}

void filter_duplicate(const morpho_tree & tree, int parent, int id, morpho_tree & output_tree){
    const morpho_node & node = tree.get_node(id);
    std::vector<int> children_ids = tree.get_children(id);

    if(node.is_of_type(morpho_node_type::neuron_branch_type)){
        const neuron_branch & b = static_cast<const neuron_branch&>(node);
        std::vector<point> points = b.get_points();
        std::vector<double> radius = b.get_radius();
        assert(radius.size() == points.size());

        point last_point(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        boost::optional<point> point_from_parent = last_point_from_parent(tree, id);
        if(point_from_parent){
            last_point = point_from_parent.get();
        }

        std::vector<point> filtered_points;
        std::vector<double> filtered_radius;
        filtered_points.reserve(points.size());
        filtered_radius.reserve(radius.size());

        for(std::size_t i =0; i < points.size(); ++i){
            if(points[i].close_to(last_point) == false){
                filtered_points.push_back(points[i]);
                filtered_radius.push_back(radius[i]);
            }
            last_point = points[i];
        }

        output_tree.add_node(parent, std::make_shared<neuron_branch>(b.get_branch_type(),
                                                             std::move(filtered_points),
                                                             std::move(filtered_radius)));
    }else{
        output_tree.copy_node(tree, id, parent);
    }

    for(auto i : children_ids){
        filter_duplicate(tree, id, i , output_tree);
    }

}

morpho_tree delete_duplicate_point_operation::apply(const morpho_tree &tree){
    morpho_tree output;
    filter_duplicate(tree, -1, 0, output);
    return output;
}


std::string delete_duplicate_point_operation::name() const{
    return "delete_duplicate_point_operation";
}




} // morpho
