#ifndef MORPHO_TREE_BITS_HPP
#define MORPHO_TREE_BITS_HPP

#include "../morpho_tree.hpp"

namespace morpho{

namespace {

inline void soma_gravity_center(const mat_points & raw_points,
                                                             point & center,
                                                             double & distance){
    namespace geo = hadoken::geometry::cartesian;
    distance =0;
    center = { 0, 0, 0 };

    for(std::size_t i = 0; i < raw_points.size1(); ++i){
        for(std::size_t j = 0; j < 3; ++j){
            center(j) += raw_points(i,j);
        }
    }


    // compute gravity center
    center /= raw_points.size1();

    // compute average distance from gravity center
    for(std::size_t i = 0; i < raw_points.size1(); ++i){
        const point p( raw_points(i,0),
                              raw_points(i,1),
                              raw_points(i,2) );

        distance += geo::distance(p, center);
    }

    distance /= raw_points.size1();
}




}


linestring branch::get_linestring() const{
    namespace geo = hadoken::geometry::cartesian;
    linestring res;
    res.reserve(get_size()+1);

    // if not first point, add the parent last point
    if(_parent != nullptr){
        if(_parent->get_type() == neuron_struct_type::soma){
            auto & soma = static_cast<branch_soma&>(*_parent);
            auto sphere = soma.get_sphere();
            geo::append(res, sphere.get_center());

        }else{
            geo::append(res, _parent->get_point(_parent->get_size()-1));
        }
    }

    for(std::size_t i =0; i < get_size(); ++i){
        geo::append(res, get_point(i));
    }
    return res;


}


inline box branch::get_bounding_box() const{
    typedef point::value_type float_type;
    const float_type max_val = std::numeric_limits<float_type>::max();

    float_type x_min(max_val), y_min(max_val), z_min(max_val);
    float_type radius(-max_val);
    float_type x_max(-max_val), y_max(-max_val), z_max(-max_val);

    if( get_size() == 0){
        std::out_of_range("impossible to get bounding box of null node");
    }

    for(std::size_t i =0; i < get_size(); ++i){
        point current_point = get_point(i);
        x_min = std::min(x_min, hg::get_x(current_point));
        y_min = std::min(y_min, hg::get_y(current_point));
        z_min = std::min(z_min, hg::get_z(current_point));

        x_max = std::max(x_max, hg::get_x(current_point));
        y_max = std::max(y_max, hg::get_y(current_point));
        z_max = std::max(z_max, hg::get_z(current_point));

        radius = std::max(radius, _radius[i]);
    }

    return box(point(x_min - radius, y_min - radius , z_min - radius), point(x_max + radius, y_max + radius , z_max + radius));
}



circle_pipe branch::get_circle_pipe() const{
    namespace geo = hadoken::geometry::cartesian;
    circle_pipe res;
    res.reserve(get_size());

    if(get_size() < 1)
        return res;

    // if not first point, add the parent last point
    if(_parent != nullptr){
        // if our parent is the soma, add a circle based on the soma sphere, oriented
        if(_parent->get_type() == neuron_struct_type::soma){
            auto & soma = static_cast<branch_soma&>(*_parent);
            auto sphere = soma.get_sphere();
            auto center = sphere.get_center();
            auto radius = sphere.get_radius();
            auto axis = center - get_point(0);
            res.emplace_back(geo::circle3d(center, radius, axis));

        }else{
            circle_pipe parent_circle_pipe = _parent->get_circle_pipe();
            if(parent_circle_pipe.size() < 1){
                throw std::runtime_error("Invalid parent circle pipe, requires at least parent to have circle pipe >= 1 circle element");
            }
            res.push_back(parent_circle_pipe.back());
        }
    }else{
        throw std::runtime_error("Unable to compute circle pipe without parent informations");
    }

    for(std::size_t i =0; i < get_size(); ++i){
        const auto  & prev_center = res.back().get_center();
        auto center = get_point(i);
        vector axis;

        // compute the axis based on the tangente if we are not the last point
        if(i < get_size() -1){
            axis = get_tangente_axis(prev_center, center, get_point(i+1));
        }else{
            axis = prev_center - center;
        }

        auto radius = _radius[i];

        if(prev_center.close_to(center)){
            namespace fmt = hadoken::format;
            fmt::scat(std::cerr, "WARNING: skip point, Duplicated point in morphology detected ", prev_center, " and ",
                                            center, " in branch ", get_id(), ",  on point id ", i, "\n");
            continue;
        }

        res.emplace_back(geo::circle3d(center, radius, axis));
    }
    return res;


}


sphere branch_soma::get_sphere() const{
    namespace fmt = hadoken::format;
    switch(get_size()){
        case 0:
            throw std::runtime_error(fmt::scat("invalid branch ", get_id(), " : null size "));
        case 1:
            return sphere(get_point(0), get_radius()[0]);
        default:{
            double radius;
            point center;
            soma_gravity_center(get_points(), center, radius);
            return sphere(center, radius);
        }
    }
}


//
// morpho tree
//



void morpho_tree::swap(morpho_tree & other){
    morpho_tree tmp = std::move(other);
    other = std::move(*this);
    *this = std::move(tmp);
}


} //morpho

#endif // MORPHO_TREE_BITS_HPP

