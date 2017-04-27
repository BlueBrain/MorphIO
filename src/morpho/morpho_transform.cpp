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

#include <tuple>
#include <memory>

#include <morpho/morpho_transform.hpp>
#include <morpho/morpho_transform_filters.hpp>






namespace morpho{


using tuple_point_radius = std::tuple<point,double>;


//
// transform operations
//
morpho_tree morpho_transform(const morpho_tree & tree, morpho_operation_chain ops){
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

// return last point of the parent branch and its radius, or empty optional if not possible (e.g root branch, no points in parent )

boost::optional< tuple_point_radius > last_point_from_parent(const morpho_tree & tree, int node_id){
    int parent_id = tree.get_parent(node_id);

    // first node, error
    if(parent_id < 0)    {
        return boost::optional<tuple_point_radius>();
    }
    const morpho_node & parent_node = tree.get_node(parent_id);

    if(parent_node.is_of_type(morpho_node_type::neuron_branch_type)){
        const neuron_branch & b = static_cast<const neuron_branch&>(parent_node);
        const std::size_t branch_size = b.get_points().size();
        if(branch_size >= 1){
            return boost::optional<tuple_point_radius>(std::make_tuple(b.get_points().back(), b.get_radius().back()));
        }
    }
    return boost::optional<tuple_point_radius>();
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
        boost::optional< tuple_point_radius > point_from_parent = last_point_from_parent(tree, id);
        if(point_from_parent){
            last_point = std::get<0>(point_from_parent.get());
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


duplicate_first_point_operation::duplicate_first_point_operation(){

}

std::string duplicate_first_point_operation::name() const {
    return "duplicate_first_point_operation";
}



void duplicate_first_point(const morpho_tree & tree, int parent, int id, morpho_tree & output_tree){
    const morpho_node & node = tree.get_node(id);
    std::vector<int> children_ids = tree.get_children(id);

    if(node.is_of_type(morpho_node_type::neuron_branch_type)){
        const neuron_branch & b = static_cast<const neuron_branch&>(node);
        std::vector<point> points = b.get_points();
        std::vector<double> radius = b.get_radius();
        assert(radius.size() == points.size());

        std::vector<point> filtered_points;
        std::vector<double> filtered_radius;

        filtered_points.reserve(points.size());
        filtered_radius.reserve(radius.size());

        boost::optional<tuple_point_radius> point_from_parent = last_point_from_parent(tree, id);
        if(point_from_parent){
            point last_point = std::get<0>(point_from_parent.get());
            double last_radius = std::get<1>(point_from_parent.get());

            if(last_point.close_to(points[0]) == false){
                filtered_points.push_back(last_point);
                filtered_radius.push_back(last_radius);
            }

        }

        std::copy(points.begin(), points.end(), std::back_inserter(filtered_points));
        std::copy(radius.begin(), radius.end(), std::back_inserter(filtered_radius));


        output_tree.add_node(parent, std::make_shared<neuron_branch>(b.get_branch_type(),
                                                             std::move(filtered_points),
                                                             std::move(filtered_radius)));
    }else{
        output_tree.copy_node(tree, id, parent);
    }

    for(auto i : children_ids){
        duplicate_first_point(tree, id, i , output_tree);
    }

}

morpho_tree duplicate_first_point_operation::apply(const morpho_tree & tree){
    morpho_tree output;
    duplicate_first_point(tree, -1, 0, output);
    return output;
}



soma_sphere_operation::soma_sphere_operation(){

}

void recursive_soma_simplify(const morpho_tree & input_tree,
                             int node_id,
                             boost::optional<std::tuple<point, double> > parent_point,
                             morpho_tree & out_tree){
    const morpho_node & node = input_tree.get_node(node_id);

    if(node.is_of_type(morpho_node_type::neuron_soma_type)){
        const neuron_soma & soma = static_cast<const neuron_soma&>(node);

        sphere res = soma.get_sphere();

        if(soma.get_line_loop().size() == 1 ){
            // already simplified
            // just setup the parent points of children
            out_tree.copy_node(input_tree, node_id, input_tree.get_parent(node_id));
            parent_point = std::make_tuple(res.get_center(), res.get_radius());
        }else {
            // simplify and setup the parent point of children

            std::shared_ptr<morpho_node> new_node(new neuron_soma(res.get_center(), res.get_radius()));
            out_tree.add_node(input_tree.get_parent(node_id), new_node);

            parent_point = std::make_tuple(res.get_center(), res.get_radius());
        }
    }else {
        if(parent_point && node.is_of_type(morpho_node_type::neuron_branch_type)){
            // if we have a parent point setup
            // use it as a first point of our branch
            // we need to refer to the new soma sphere center

            const neuron_branch & branch = static_cast<const neuron_branch&>(node);
            const std::vector<point> & origin_points = branch.get_points();
            const std::vector<double> & origin_radius = branch.get_radius();

            std::vector<point> points;
            std::vector<double> radius;
            points.reserve(branch.get_number_points() +1);
            radius.reserve(branch.get_number_points() +1);

            points.push_back(std::get<0>(parent_point.get()));
            radius.push_back(std::get<1>(parent_point.get()));

            std::copy(origin_points.begin(), origin_points.end(), std::back_inserter(points));
            std::copy(origin_radius.begin(), origin_radius.end(), std::back_inserter(radius));

            out_tree.add_node(input_tree.get_parent(node_id), std::shared_ptr<morpho_node>(
               new neuron_branch(branch.get_branch_type(), std::move(points), std::move(radius))
            ));
        }else{
            out_tree.copy_node(input_tree, node_id, input_tree.get_parent(node_id));
        }

        parent_point.reset();
    }

    const std::vector<int> children = input_tree.get_children(node_id);
    for(auto child : children){
        recursive_soma_simplify(input_tree, child, parent_point, out_tree);
    }
}

morpho_tree soma_sphere_operation::apply(const morpho_tree &tree){
    morpho_tree res;
    recursive_soma_simplify(tree, 0, boost::optional<std::tuple<point, double> >(), res);
    return res;
}


std::string soma_sphere_operation::name() const{
    return "soma_sphere_operation";
}


} // morpho
