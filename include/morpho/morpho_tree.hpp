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
#ifndef MORPHO_TREE_HPP
#define MORPHO_TREE_HPP


#include <vector>
#include <memory>
#include <bitset>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <hadoken/geometry/geometry.hpp>
#include <hadoken/format/format.hpp>


namespace morpho{

namespace hg = hadoken::geometry::cartesian;


typedef boost::numeric::ublas::matrix<double> mat_points;
typedef boost::numeric::ublas::matrix_range<mat_points> mat_range_points;

typedef boost::numeric::ublas::vector<double> vec_double;
typedef boost::numeric::ublas::vector_range<vec_double> vec_double_range;

using range =  boost::numeric::ublas::range;

using point = hg::point3d;

using vector = hg::vector3d;

using linestring = hg::linestring3d;

using sphere = hadoken::geometry::cartesian::sphere3d;

using circle_pipe = std::vector<hg::circle3d>;

using box = hg::box3d;

class branch;
class morpho_tree;


/// branch type
enum class branch_type{
    soma =0x00,
    axon = 0x01,
    dentrite_basal = 0x02,
    dentrite_apical = 0x03,
    unknown = 0x04
};


///
/// \brief base node element for a morphology node
///
///  A common node can be a dentrite, an axon or a soma
///
class morpho_node{
public:
    inline morpho_node(branch_type my_node_type) : _my_type(my_node_type){}

    inline virtual ~morpho_node(){};

    inline branch_type get_type() const{
        return _my_type;
    }

protected:
    branch_type _my_type;

private:
    morpho_node(const morpho_node&) = delete;
    morpho_node & operator =(const morpho_node &) = delete;
};

///
/// \brief generic container for a branch of a morphology
///
class branch : public morpho_node{
public:

    inline branch(branch_type type_b) : morpho_node(type_b), _parent(nullptr), _id(0) {}

    inline virtual ~branch(){}



    inline void set_points(mat_points && points, vec_double && distances){
        _points = std::move(points);
        _distances = std::move(distances);
    }

    ///
    /// \brief  get all the points associated to the branch
    ///
    ///  the result does not contain the starting connected point coming from the previous branch
    ///
    /// \return matrix of all the points associated to the branch, one point per row
    ///
    inline const mat_points & get_points() const{
        return _points;
    }

    ///
    /// \brief get_distances
    /// \return
    ///
    inline const vec_double & get_distances() const{
        return _distances;
    }

    ///
    /// \brief get_size
    /// \return number of point composing the branch
    ///
    inline std::size_t get_size() const{
        assert(_distances.size() == _points.size1());
        return _distances.size();
    }

    ///
    /// \param id
    /// \return return point in the branch for a given id
    ///
    ///  the id msut be inferior to the branch size
    ///
    inline point get_point(const std::size_t id) const{
        namespace fmt = hadoken::format;
        if(id >= get_size()){
            std::out_of_range(fmt::scat("id ", id, " out of range "));
        }
        return point(_points(id, 0),
                      _points(id, 1),
                      _points(id, 2));
    }

    inline virtual box get_bounding_box() const{
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

            radius = std::max(radius, _distances[i]);
        }

        return box(point(x_min - radius, y_min - radius , z_min - radius), point(x_max + radius, y_max + radius , z_max + radius));
    }


    ///
    /// \brief get_segment_bounding_box
    /// \param pos
    /// \return bounding box of segment n of the given branch
    ///
    inline box get_segment_bounding_box(std::size_t n) const{
        if(n >= get_size()){
            throw std::out_of_range(hadoken::format::scat("segment ", n, " is out of bound"));
        }

        auto p1 = get_point(n);
        auto p2 = get_point(n+1);
        double radius = std::max(_distances[n], _distances[n+1]);

        const point p_min(std::min(get_x(p1), get_x(p2)),
                    std::min(get_y(p1), get_y(p2)),
                    std::min(get_z(p1), get_z(p2)));

        const point p_max(std::max(get_x(p1), get_x(p2)),
                    std::max(get_y(p1), get_y(p2)),
                    std::max(get_z(p1), get_z(p2)));

        const point offset_radius(radius, radius, radius);


        return box(p_min - offset_radius, p_max + offset_radius);
    }

    ///
    /// \brief get_linestring
    /// \return
    ///
    inline linestring get_linestring() const;

    ///
    /// \brief get_circle_pipe
    /// \return
    ///
    inline circle_pipe get_circle_pipe() const;

    inline const std::vector<std::size_t> & get_childrens() const{
        return _childrens;
    }

    inline std::size_t get_parent() const{
        if(_parent == nullptr)
            return 0;
        return _parent->get_id();

    }

    inline std::size_t get_id() const{
        return _id;
    }

private:

    branch* _parent;

    std::size_t _id;

    mat_points _points;

    vec_double _distances;

    std::vector<std::size_t > _childrens;

    friend class morpho_tree;
};


///
/// \brief soma branch type
///
class branch_soma : public branch{
public:
    inline branch_soma() : branch(branch_type::soma) {}
    inline virtual ~branch_soma(){}

   inline  sphere get_sphere() const;


   inline virtual box get_bounding_box() const{
       auto s = get_sphere();
       auto radius = s.get_radius();
       auto center = s.get_center();


       return box( center - point(radius, radius, radius),
                   center + point (radius, radius, radius));
   }


private:

};


///
/// \brief container for an entire morphology tree
///
class morpho_tree : private boost::noncopyable{
public:
    /// flags
    static constexpr int point_soma_flag = 0x01;
    static constexpr int no_dup_point_flag = 0x02;


    inline morpho_tree() : _branches(), _flags(0) {}
    inline virtual ~morpho_tree(){}

    morpho_tree(morpho_tree && other){
        _branches = std::move(other._branches);
        _flags = other._flags;
    }


    ///
    /// \brief set the root element of the tree
    /// \param root_elem
    ///
    inline std::size_t set_root(std::unique_ptr<branch> && root_elem){
        root_elem->_id = 0;
        root_elem->_parent = nullptr;
        _branches.emplace_back(std::move(root_elem));
        return 0;
    }



    ///
    /// \brief insert a new child branch intro the tree, as children of parent_id branch
    /// \param parent_id
    /// \param children
    /// \return  id of the branch
    ///
    inline std::size_t add_child(std::size_t parent_id, std::unique_ptr<branch> && children){
        if(parent_id >= _branches.size()){
            std::runtime_error("Invalid parent id ");
        }

        const std::size_t my_id = _branches.size();

        children->_id = my_id;
        children->_parent = &get_branch(parent_id);

        _branches.emplace_back(std::move(children));
        _branches[parent_id]->_childrens.push_back(my_id);
        return my_id;
    }


    ///
    /// \brief get branch for a given id
    /// \param id_branch
    /// \return
    ///
    inline branch & get_branch(std::size_t id_branch){
        if(id_branch >= _branches.size()){
            std::runtime_error("Invalid branch id ");
        }
        return *(_branches[id_branch]);
    }


    inline box get_bounding_box() const{
        typedef point::value_type float_type;
        const float_type max_val = std::numeric_limits<float_type>::max();

        float_type x_min(max_val), y_min(max_val), z_min(max_val);
        float_type x_max(-max_val), y_max(-max_val), z_max(-max_val);

        box res(point(x_min, y_min, z_min), point(x_max, y_max, z_max));

        std::for_each(_branches.begin(), _branches.end(), [&](const std::unique_ptr<branch> &  b_ptr){
            box branch_box = b_ptr->get_bounding_box();
            res = merge_box(branch_box, res);
        });

        return res;
    }

    ///
    /// \brief get number of branch
    /// \return
    ///
    inline std::size_t get_tree_size() const{
        return _branches.size();
    }

    inline void add_flag(int flag){
        _flags |= flag;
    }

    inline int get_flags() const{
        return _flags;
    }


private:
    std::vector<std::unique_ptr<branch> > _branches;
    int _flags;

};

} //morpho

#include "bits/morpho_tree_bits.hpp"

#endif // MORPHO_TREE_HPP

