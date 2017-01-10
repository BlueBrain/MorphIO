#include "morpho_spatial.hpp"


namespace morpho{

class spatial_index_impl{
public:


    std::vector<std::shared_ptr<morpho_tree> > morphos;
};

spatial_index::spatial_index() : _pimpl(new spatial_index_impl())
{

}


void spatial_index::add_morpho_tree(const std::shared_ptr<morpho_tree> &tree){
    _pimpl->morphos.push_back(tree);
}


} // morpho

