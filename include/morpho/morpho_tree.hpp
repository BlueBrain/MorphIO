#ifndef MORPHO_TREE_HPP
#define MORPHO_TREE_HPP


#include <vector>
#include <memory>
#include <bitset>

#include <boost/noncopyable.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>


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
    typedef boost::numeric::ublas::vector<double> vec_double;

    typedef std::array<double, 3> point;



    inline branch(branch_type type_b) : _type(type_b), _id(0), _parent_id(0) {}

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


    inline branch_type get_type() const{
        return _type;
    }

    inline std::size_t get_parent_id() const{
        return _parent_id;
    }

    inline const std::vector<std::size_t> & get_childrens() const{
        return _childrens;
    }

    inline std::size_t get_parent() const{
        return _parent_id;
    }

    inline std::size_t get_id() const{
        return _id;
    }

private:
    branch_type _type;

    std::size_t _id, _parent_id;

    mat_points _points;

    vec_double _distances;

    std::vector<std::size_t > _childrens;

    friend class morpho_tree;
};


///
/// \brief soma branch type
///
class soma : public branch{
public:
    inline soma() : branch(branch_type::soma) {}
    inline virtual ~soma(){}


private:

};


///
/// \brief container for an entire morphology tree
///
class morpho_tree : private boost::noncopyable{
public:
    /// flags
    static constexpr int point_soma = 0x01;

    inline morpho_tree() {}
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
        root_elem->_parent_id = 0;
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

        const std::size_t my_id = _branches.size();;

        children->_id = my_id;
        children->_parent_id = parent_id;

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

    inline void add_flag(const std::bitset<64> & flag){
        _flags |= flag;
    }

    inline std::bitset<64> get_flags() const{
        return _flags;
    }


private:
    std::vector<std::unique_ptr<branch> > _branches;
    std::bitset<64> _flags;

};

} //morpho

#endif // MORPHO_TREE_HPP

