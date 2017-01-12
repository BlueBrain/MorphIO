#include "morpho_spatial.hpp"

#include <tuple>

#include <hadoken/containers/small_vector.hpp>
#include <boost/geometry/index/rtree.hpp>
 #include <boost/geometry/index/indexable.hpp>

namespace morpho{

namespace bgi = boost::geometry::index;

class spatial_index_impl{
public:

    // define the indexed element
    // contain
    // 1- bounding box
    // 2- morpho id
    // 3- branch id
    // 4- segment id
    // 5- 0 -> soma, 1-> segment, 2 -> segment junction
    // please note that all ids are specific to a given morpho tree
    // not to a file format
    using indexed_box = std::tuple<box, int, int, int, int>;

    // R*-tree
    bgi::rtree<indexed_box, bgi::rstar<16> > sp_index;

    // all registered morpho tree
    std::vector<std::shared_ptr<morpho_tree> > morphos;
};

spatial_index::spatial_index() : _pimpl(new spatial_index_impl())
{

}

spatial_index::~spatial_index(){

}


void spatial_index::add_morpho_tree(const std::shared_ptr<morpho_tree> &tree){
    if(tree->get_tree_size() ==0){
        return;
    }

    const std::size_t morpho_position = _pimpl->morphos.size();
    _pimpl->morphos.push_back(tree);


    // add soma bounding box
    box soma_box = tree->get_branch(0).get_bounding_box();
    _pimpl->sp_index.insert(std::make_tuple(soma_box, morpho_position, 0, 0, 0));
    //std::cout << "soma box" << soma_box.min_corner() << " " << soma_box.max_corner() << "\n";

    // insert boundinx box for each segment
    for(std::size_t i =1; i < tree->get_tree_size(); ++i){
        branch & current_branch = tree->get_branch(i);

        for(std::size_t j =0; j < current_branch.get_size() -1; ++j){
            auto segment_box = current_branch.get_segment_bounding_box(j);
            _pimpl->sp_index.insert(std::make_tuple(segment_box, morpho_position, i, j, 1));

            auto junction_box = current_branch.get_junction_sphere_bounding_box(j);
            _pimpl->sp_index.insert(std::make_tuple(junction_box, morpho_position, i, j, 2));
        }
    }
}

inline bool point_is_in_sphere(const sphere & s, const point & p1){
    return (hg::distance(p1, s.get_center()) <= s.get_radius());
}

inline bool point_is_in_truncated_cones(const cone & c, const point & p1){
    const point x1_cone = c.get_center<0>();
    const point x2_cone = c.get_center<1>();

    const double x1_radius = c.get_radius<0>();
    const double x2_radius = c.get_radius<1>();

   /* std::cout << "test cone" << x1_cone << "," << x1_radius << " " << x2_cone
              << "," << x2_radius << std::endl;*/

    const vector v_axis = hg::normalize(vector(x2_cone - x1_cone));
    const vector v_to_point(p1 - x1_cone);
    double lenght = hg::distance(x2_cone, x1_cone);

    double dotprodv = hg::dot_product(v_to_point, v_axis);
    if( dotprodv < 0 || dotprodv > lenght){
        return false;
    }

    double radius_at_projection = (dotprodv / lenght * (x2_radius - x1_radius)) + x1_radius;

    const point proj_point = v_axis * dotprodv + x1_cone;

    const bool is_inside = ( hg::distance(proj_point, p1) < radius_at_projection);
   // std::cout << " inside " << ((is_inside)?"TRUE":"FALSE") << "\n";
    return is_inside;
}

bool spatial_index::is_within(const point & p) const{
    box fake_box(p - point(0.01, 0.01, 0.01), p + point(0.01, 0.01, 0.01));

    hadoken::containers::small_vector<spatial_index_impl::indexed_box, 64> res;
    _pimpl->sp_index.query(bgi::contains(fake_box), std::back_inserter(res));

    if(res.empty()){
        // do not match anything inside the R-tree
        return false;
    }

    // check if individual element intersect
    for(auto it = res.begin(); it < res.end(); ++it){
        const int n_morpho = std::get<1>(*it);
        const int n_branch = std::get<2>(*it);
        const int n_segment = std::get<3>(*it);
        const int indexed_type = std::get<4>(*it);

        switch(indexed_type){
            case 0:{
                branch_soma & soma = static_cast<branch_soma&>(_pimpl->morphos[n_morpho]->get_branch(n_branch));
                if(point_is_in_sphere(soma.get_sphere(), p)){
                    return true;
                }
                break;
            }

            case 1:{
                //return true;
                branch & my_branch = _pimpl->morphos[n_morpho]->get_branch(n_branch);
                if(point_is_in_truncated_cones(my_branch.get_segment(n_segment), p)){
                    return true;
                }
                break;
            }

            case 2:{
                branch & my_branch = _pimpl->morphos[n_morpho]->get_branch(n_branch);
                if(point_is_in_sphere(my_branch.get_junction(n_segment), p)){
                    return true;
                }
                break;
            }

            default:{
                // unmanaged case
                std::cerr << "Error: unmanaged indexed type, skip \n";
                return false;
            }
        }

    };

    return false;
}


} // morpho

