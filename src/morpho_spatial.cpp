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
    // please note that all ids are specific to a given morpho tree
    // not to a file format
    using indexed_box = std::tuple<box, int, int, int>;

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
    _pimpl->sp_index.insert(std::make_tuple(soma_box, morpho_position, 0, 0));
    //std::cout << "soma box" << soma_box.min_corner() << " " << soma_box.max_corner() << "\n";

    // insert boundinx box for each segment
    for(std::size_t i =1; i < tree->get_tree_size(); ++i){
        branch & current_branch = tree->get_branch(i);

        for(std::size_t j =0; j < current_branch.get_size() -1; ++j){
            auto segment_box = current_branch.get_segment_bounding_box(j);
            _pimpl->sp_index.insert(std::make_tuple(segment_box, morpho_position, i, j));
        }
    }
}


bool spatial_index::is_within(point p) const{
    box fake_box(p - point(0.01, 0.01, 0.01), p + point(0.01, 0.01, 0.01));

    hadoken::containers::small_vector<spatial_index_impl::indexed_box, 64> res;
    _pimpl->sp_index.query(bgi::contains(fake_box), std::back_inserter(res));
    return !(res.empty());
}


} // morpho

