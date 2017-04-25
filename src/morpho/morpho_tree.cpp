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


#include <morpho/morpho_tree.hpp>


namespace morpho{



// return the tangeante of a 3 points linestring in point 2
inline vector get_tangente_axis(const point & p1, const point & p2, const point & p3){
    namespace geo = hadoken::geometry::cartesian;
    vector v1 = p2 - p1, v2 = p3 - p2;

    return v1 + v2;
}

struct morpho_node::morpho_node_internal{


};








morpho_node::morpho_node() : _dptr(new morpho_node_internal()){

}


morpho_node::morpho_node(const morpho_node &other): _dptr(new morpho_node_internal(*(other._dptr))){

}


morpho_node::~morpho_node(){

}


neuron_node_3d::neuron_node_3d(neuron_struct_type my_node_type) : _my_type(my_node_type){

}

neuron_node_3d::~neuron_node_3d(){

}


bool neuron_node_3d::is_of_type(morpho_node_type mtype) const{
    switch(mtype){
        case morpho_node_type::neuron_node_3d_type:
            return true;

        default:
            return false;
    }
}


//
// neuron_branch
//


struct neuron_branch::neuron_branch_internal{
    inline neuron_branch_internal(std::vector<point> && p, std::vector<double> && r) :
    points(std::move(p)), radius(std::move(r)){
        if(points.size() < 1){
            throw std::invalid_argument("a neuron branch should have at least one point");
        }
        if(points.size() != radius.size()){
            throw std::invalid_argument(" points and radius vector for neuron branch should have the same size ");
        }

    }

    std::vector<point> points;
    std::vector<double> radius;
};


neuron_branch::neuron_branch(neuron_struct_type neuron_type, std::vector<point> && points, std::vector<double> && radius) :
    neuron_node_3d(neuron_type),
    _dptr(new neuron_branch_internal(std::move(points), std::move(radius))){

}

neuron_branch::neuron_branch(const neuron_branch &other) :
    neuron_node_3d(other),
    _dptr( new neuron_branch_internal(*(other._dptr)))
{

}

neuron_branch::~neuron_branch(){}


std::size_t neuron_branch::get_number_points() const{
    return _dptr->points.size();
}

bool neuron_branch::is_of_type(morpho_node_type mtype) const{
    switch(mtype){
        case morpho_node_type::neuron_node_3d_type:
        case morpho_node_type::neuron_branch_type:
            return true;
        default:
            return false;
    }
}


const std::vector<point> & neuron_branch::get_points() const{
    return _dptr->points;
}

const std::vector<double> & neuron_branch::get_radius() const{
    return _dptr->radius;
}

cone neuron_branch::get_segment(std::size_t n) const{

    if(n >= get_number_points()){
        throw std::out_of_range(hadoken::format::scat("segment ", n, " is out of bound"));
    }

    return cone(_dptr->points[n], _dptr->radius[n], _dptr->points[n+1], _dptr->radius[n+1]);
}


box neuron_branch::get_bounding_box() const{

    if( get_number_points() == 0){
        std::logic_error("impossible to get bounding box of null node");
    }

    typedef point::value_type float_type;
    const float_type max_val = std::numeric_limits<float_type>::max();

    float_type x_min(max_val), y_min(max_val), z_min(max_val);
    float_type radius(-max_val);
    float_type x_max(-max_val), y_max(-max_val), z_max(-max_val);


    std::size_t i = 0;
    for(const auto & current_point : get_points()){
        i++;

        x_min = std::min(x_min, hg::get_x(current_point));
        y_min = std::min(y_min, hg::get_y(current_point));
        z_min = std::min(z_min, hg::get_z(current_point));

        x_max = std::max(x_max, hg::get_x(current_point));
        y_max = std::max(y_max, hg::get_y(current_point));
        z_max = std::max(z_max, hg::get_z(current_point));

        radius = std::max(radius, get_radius()[i]);
    }

    return box(point(x_min - radius, y_min - radius , z_min - radius), point(x_max + radius, y_max + radius , z_max + radius));
}



box neuron_branch::get_segment_bounding_box(std::size_t n) const{
    if(n >= get_number_points()){
        throw std::out_of_range(hadoken::format::scat("segment ", n, " is out of bound"));
    }

    auto p1 = get_points()[n];
    auto p2 = get_points()[n+1];

    double radius = std::max(get_radius()[n], get_radius()[n+1]);

    const point p_min(std::min(get_x(p1), get_x(p2)),
                std::min(get_y(p1), get_y(p2)),
                std::min(get_z(p1), get_z(p2)));

    const point p_max(std::max(get_x(p1), get_x(p2)),
                std::max(get_y(p1), get_y(p2)),
                std::max(get_z(p1), get_z(p2)));

    const point offset_radius(radius, radius, radius);


    return box(p_min - offset_radius, p_max + offset_radius);
}


sphere neuron_branch::get_junction(std::size_t n) const{
    if(n >= get_number_points()){
        throw std::out_of_range(hadoken::format::scat("segment ", n, " is out of bound"));
    }

    return sphere(get_points()[n+1], get_radius()[n+1]);
}


box neuron_branch::get_junction_sphere_bounding_box(std::size_t n) const{
    return hg::envelope_sphere_return<box, sphere>(get_junction(n));
}


linestring neuron_branch::get_linestring() const{
    namespace geo = hadoken::geometry::cartesian;
    linestring res;

    for(const auto & point : get_points()){
        hg::append(res, point);
    }

    return res;
}


circle_pipe neuron_branch::get_circle_pipe() const{
    namespace geo = hadoken::geometry::cartesian;

    circle_pipe res;
    if(get_number_points() < 2){
        std::out_of_range("a circle pipe can not be constructed on a branch < 2 points");
    }

    res.reserve(get_number_points());


    for(std::size_t i =0; i < get_number_points(); ++i){

        auto center = get_points()[i];
        vector axis;

        // compute the axis based on the tangente with the last point
        if(i  == 0){
            auto next_center = get_points()[i+1];
            axis = next_center - center;
        } if(i == get_number_points() -1){
            auto prev_center = get_points()[i-1];
            axis = center - prev_center;
        }else{
            axis = get_tangente_axis(get_points()[i-1], get_points()[i], get_points()[i+1]);
        }

        auto radius = get_radius()[i];


        res.emplace_back(geo::circle3d(center, radius, axis));
    }
    return res;
}


//
// neuron_soma
//

inline void soma_gravity_center(const std::vector<point> & points,
                                                             point & center,
                                                             double & distance){
    namespace geo = hadoken::geometry::cartesian;
    distance = 0;
    center = { 0, 0, 0 };

    for(const point & p : points){
        center += p;
    }


    // compute gravity center
    center /= points.size();

    // compute average distance from gravity center
    for(const point & p : points){

        distance += geo::distance(p, center);
    }

    distance /= points.size();
}


enum class _morpho_soma_type{
    soma_sphere = 0x00,
    soma_line_loop = 0x01
};

struct neuron_soma::neuron_soma_intern{
    inline neuron_soma_intern(std::vector<point> && p) :
        radius(0),
        points(std::move(p)),
        soma_type(_morpho_soma_type::soma_line_loop){}

    inline neuron_soma_intern(const point & p, double r) :
        radius(r),
        points({p}),
        soma_type(_morpho_soma_type::soma_sphere){}

    double radius;
    std::vector<point> points;
    _morpho_soma_type soma_type;
};


neuron_soma::neuron_soma(std::vector<point> && points) :
    neuron_node_3d(neuron_struct_type::soma),
    _dptr(new neuron_soma_intern(std::move(points)))
{

}


neuron_soma::neuron_soma(const point & p, double radius) :
    neuron_node_3d(neuron_struct_type::soma),
    _dptr(new neuron_soma_intern(p, radius))
{

}


neuron_soma::~neuron_soma(){

}


bool neuron_soma::is_of_type(morpho_node_type mtype) const{
    switch(mtype){
        case morpho_node_type::neuron_node_3d_type:
        case morpho_node_type::neuron_soma_type:
            return true;
        default:
            return false;
    }
}

sphere neuron_soma::get_sphere() const{
    namespace fmt = hadoken::format;
    switch(_dptr->soma_type){
        case _morpho_soma_type::soma_sphere:
            return sphere(_dptr->points[0], _dptr->radius);

        case _morpho_soma_type::soma_line_loop:{
            double radius;
            point center;
            soma_gravity_center(get_line_loop(), center, radius);
            return sphere(center, radius);
        }
       default:{
              throw std::runtime_error(fmt::scat("invalid soma type"));
        }
    }
}

box neuron_soma::get_bounding_box() const{
    auto s = get_sphere();
    auto box_sphere = hg::envelope_sphere_return<box, sphere>(s);

    typedef point::value_type float_type;

    const point min_corner = box_sphere.min_corner();
    const point max_corner = box_sphere.max_corner();

    float_type x_min(min_corner(0)), y_min(min_corner(1)), z_min(min_corner(2));

    float_type x_max(max_corner(0)), y_max(max_corner(1)), z_max(max_corner(2));


    for(const auto & current_point : _dptr->points){

        x_min = std::min(x_min, hg::get_x(current_point));
        y_min = std::min(y_min, hg::get_y(current_point));
        z_min = std::min(z_min, hg::get_z(current_point));

        x_max = std::max(x_max, hg::get_x(current_point));
        y_max = std::max(y_max, hg::get_y(current_point));
        z_max = std::max(z_max, hg::get_z(current_point));

    }

    return box(point(x_min, y_min, z_min), point(x_max, y_max, z_max));
}

const std::vector<point> & neuron_soma::get_line_loop() const{
    return _dptr->points;
}

//
// morpho_tree
//
struct morpho_tree::morpho_tree_intern{
    std::vector<std::shared_ptr<morpho_node>> nodes;
    std::vector<int> parents;
};

morpho_tree::morpho_tree() : _dptr(new morpho_tree_intern()){

}

morpho_tree::morpho_tree(morpho_tree && other) : _dptr(nullptr){
    std::swap(_dptr, other._dptr);
}

morpho_tree::morpho_tree(const morpho_tree & other) :  _dptr(new morpho_tree_intern(*(other._dptr))){

}


morpho_tree::~morpho_tree(){
}


std::size_t morpho_tree::get_tree_size() const{
    return _dptr->nodes.size();
}


box morpho_tree::get_bounding_box() const{
    typedef point::value_type float_type;
    const float_type max_val = std::numeric_limits<float_type>::max();

    float_type x_min(max_val), y_min(max_val), z_min(max_val);
    float_type x_max(-max_val), y_max(-max_val), z_max(-max_val);

    box res(point(x_min, y_min, z_min), point(x_max, y_max, z_max));

    std::for_each(_dptr->nodes.begin(), _dptr->nodes.end(), [&](const std::shared_ptr<morpho_node> &  nd){
        box branch_box = nd->get_bounding_box();
        res = merge_box(branch_box, res);
    });

    return res;
}

void morpho_tree::swap(morpho_tree &other){
    std::swap(_dptr, other._dptr);
}

const morpho_node & morpho_tree::get_node(int id) const{
    if(id < 0 ||  std::size_t(id) >= _dptr->nodes.size()){
        throw std::out_of_range("Invalid node in morpholo tree, out of range");
    }

    return *(_dptr->nodes[id]);
}

int morpho_tree::get_parent(int id) const{
    if(id < 0 ||  std::size_t(id) >= _dptr->nodes.size()){
        throw std::out_of_range("Invalid node in morpholo tree, out of range");
    }

    return _dptr->parents[id];
}

int morpho_tree::add_node(int parent_id, const std::shared_ptr<morpho_node> & new_node){

    if(parent_id != -1 && std::size_t(parent_id) >= _dptr->nodes.size()){
        throw std::out_of_range("Invalid parent id, should match id of existing node or be -1 for new root");
    }

    // insert new root
    if(parent_id < 0 ){
        if(_dptr->nodes.size() > 0){
            throw std::logic_error("Root node of morpho tree should always be inserted first");
        }
        _dptr->nodes.push_back(new_node);
        _dptr->parents.push_back(-1);
        return 0;
    }

    _dptr->nodes.push_back(new_node);
    _dptr->parents.push_back(parent_id);
    return _dptr->nodes.size()-1;
}


int morpho_tree::copy_node(const morpho_tree &other, int id, int new_parent_id){
     if(id < 0 || id >= (int)other.get_tree_size()){
         throw std::logic_error("Invalid node id for copy tree operation");
     }
     if(new_parent_id < 0 && _dptr->nodes.size() > 0){
         throw std::logic_error("Only one root without parent can be defined");
     }

     _dptr->nodes.push_back(other._dptr->nodes[id]);
     _dptr->parents.push_back(new_parent_id);
    return _dptr->nodes.size()-1;
}


std::vector<int> morpho_tree::get_children(int id) const{
    std::vector<int> res;
    res.reserve(_dptr->nodes.size());

    for(std::size_t i = 0; i < _dptr->nodes.size(); ++i){
        if ( _dptr->parents[i] == id ) {
            res.push_back(i);
        }
    }

    return res;
}


std::vector<morpho_node const *> morpho_tree::get_all_nodes() const{
    std::vector<morpho_node const *> res;
    res.reserve(_dptr->nodes.size());

    for(auto & node : _dptr->nodes){
        res.push_back(node.get());
    }
    return res;
}


std::vector<unsigned int> morpho_tree::find_nodes(neuron_struct_type mtype) const {
    std::vector<unsigned int> res;

    unsigned int cur_idx = 0;
    for(auto & node : _dptr->nodes){
        if(node->is_of_type(morpho_node_type::neuron_node_3d_type)){
            neuron_node_3d *nodex = static_cast<neuron_node_3d*>(node.get());
            if(nodex->get_branch_type() == mtype) {
                res.push_back(cur_idx);
            }
        }
        cur_idx ++;
    }
    return res;
}


neuron_soma const* morpho_tree::get_soma() const {
    if (_dptr->nodes.size() > 0) {
        return dynamic_cast<neuron_soma*>(_dptr->nodes[0].get());
    }
    return nullptr;
}


morpho_tree& morpho_tree::operator = (morpho_tree && other){
    if(&other == this){
        return *this;
    }

    swap(other);
    other._dptr.reset();
    return *this;
}


morpho_tree& morpho_tree::operator =(const morpho_tree & other){
    if(&other == this){
        return *this;
    }

    morpho_tree new_tree(other);
    swap(new_tree);
    return *this;
}

} //morpho


