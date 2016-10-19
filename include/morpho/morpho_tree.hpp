#ifndef MORPHO_TREE_HPP
#define MORPHO_TREE_HPP


#include <vector>
#include <memory>
#include <bitset>

#include <boost/noncopyable.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <hadoken/geometry/geometry.hpp>
#include <hadoken/format/format.hpp>


namespace morpho{


class branch;
class morpho_tree;


/// branch type
enum class branch_type{
    soma =0x00,
    axon = 0x01,
    dentrite_basal = 0x02,
    dentrite_apical = 0x03
};


///
/// \brief generic container for a branch of a morphology
///
class branch : private boost::noncopyable{
public:

    typedef boost::numeric::ublas::matrix<double> mat_points;
    typedef boost::numeric::ublas::matrix_range<mat_points> mat_range_points;

    typedef boost::numeric::ublas::vector<double> vec_double;
    typedef boost::numeric::ublas::vector_range<vec_double> vec_double_range;

    using range =  boost::numeric::ublas::range;
    using point = hadoken::geometry::cartesian::point3d;
    using linestring = hadoken::geometry::cartesian::linestring3d;



    inline branch(branch_type type_b) : _type(type_b), _parent(nullptr), _id(0) {}

    inline virtual ~branch(){}



    inline void set_points(mat_points && points, vec_double && distances){
        _points = std::move(points);
        _distances = std::move(distances);
    }

    inline const mat_points & get_points() const{
        return _points;
    }

    inline const vec_double & get_distances() const{
        return _distances;
    }

    inline std::size_t get_size() const{
        assert(_distances.size() == _points.size1());
        return _distances.size();
    }

    inline point get_point(const std::size_t id) const{
        namespace fmt = hadoken::format;
        if(id >= get_size()){
            std::out_of_range(fmt::scat("id ", id, " out of range "));
        }
        return point(_points(id, 0),
                      _points(id, 1),
                      _points(id, 2));
    }

    inline linestring get_linestring() const;

    inline branch_type get_type() const{
        return _type;
    }

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
    branch_type _type;

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
    using sphere = hadoken::geometry::cartesian::sphere3d;

    inline branch_soma() : branch(branch_type::soma) {}
    inline virtual ~branch_soma(){}


   inline  sphere get_sphere() const;


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

