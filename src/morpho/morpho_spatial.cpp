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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 **/
#include <morpho/morpho_spatial.hpp>

#include <tuple>

#include <hadoken/containers/small_vector.hpp>
#include <hadoken/math/math_floating_point.hpp>

#include <boost/geometry/index/indexable.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace morpho {

namespace bgi = boost::geometry::index;

class spatial_index_impl {
  public:
    // define the indexed element
    // contain
    // 1- bounding box
    // 2- morpho id
    // 3- section id
    // 4- segment id
    // 5- 0 -> soma, 1-> segment, 2 -> segment junction
    // please note that all ids are specific to a given morpho tree
    // not to a file format
    using indexed_box = std::tuple<box, int, int, int, int>;

    // R*-tree
    bgi::rtree<indexed_box, bgi::rstar<16>> sp_index;

    // all registered morpho tree
    std::vector<std::shared_ptr<morpho_tree>> morphos;
};

spatial_index::spatial_index() : _pimpl(new spatial_index_impl()) {}

spatial_index::~spatial_index() {}

void spatial_index::add_morpho_tree(const std::shared_ptr<morpho_tree>& tree) {
    if (tree->get_tree_size() == 0) {
        return;
    }

    const std::size_t morpho_position = _pimpl->morphos.size();
    _pimpl->morphos.push_back(tree);

    // add soma bounding box

    // std::cout << "soma box" << soma_box.min_corner() << " " <<
    // soma_box.max_corner() << "\n";

    // insert boundinx box for each segment
    for (std::size_t i = 0; i < tree->get_tree_size(); ++i) {
        const morpho_node& current_node = tree->get_node(i);

        if (current_node.is_of_type(morpho_node_type::neuron_soma_type)) {
            const neuron_soma& soma =
                static_cast<const neuron_soma&>(current_node);

            box soma_box = soma.get_bounding_box();
            const spatial_index_impl::indexed_box soma_elem =
                std::make_tuple(soma_box, morpho_position, 0, 0, 0);

            const std::array<spatial_index_impl::indexed_box, 1> all_elems(
                {soma_elem});
            _pimpl->sp_index.insert(all_elems.begin(), all_elems.end());

        } else if (current_node.is_of_type(
                       morpho_node_type::neuron_section_type)) {
            const neuron_section& current_section =
                static_cast<const neuron_section&>(current_node);

            for (std::size_t j = 0; j < current_section.get_number_points() - 1;
                 ++j) {
                auto segment_box = current_section.get_segment_bounding_box(j);
                const spatial_index_impl::indexed_box segment_elem =
                    std::make_tuple(segment_box, morpho_position, i, j, 1);

                auto junction_box =
                    current_section.get_junction_sphere_bounding_box(j);
                const spatial_index_impl::indexed_box junction_elem =
                    std::make_tuple(junction_box, morpho_position, i, j, 2);

                const std::array<spatial_index_impl::indexed_box, 2> all_elems(
                    {segment_elem, junction_elem});
                _pimpl->sp_index.insert(all_elems.begin(), all_elems.end());
            }
        } else {
            std::cerr << " skip morphology element " << i << " : unknown type "
                      << std::endl;
        }
    }
}

inline bool point_is_in_sphere(const sphere& s, const point& p1) {
    return (hg::distance(p1, s.get_center()) <= s.get_radius());
}

inline bool point_is_in_truncated_cones(const cone& c, const point& p1) {
    using namespace hadoken::math;

    const point x1_cone = c.get_center<0>();
    const point x2_cone = c.get_center<1>();

    const double x1_radius = c.get_radius<0>();
    const double x2_radius = c.get_radius<1>();

    /* std::cout << "test cone" << x1_cone << "," << x1_radius << " " << x2_cone
               << "," << x2_radius << std::endl;*/

    const vector v_axis_diff = vector(x2_cone - x1_cone);
    double length = hg::norm(v_axis_diff);

    if (close_to_abs<double>(length, 0.0)) {
        // null segment
        return false;
    }

    const vector v_axis = v_axis_diff * (1.0 / length);
    const vector v_to_point(p1 - x1_cone);

    double dotprodv = hg::dot_product(v_to_point, v_axis);
    if (dotprodv < 0 || dotprodv > length) {
        return false;
    }

    double radius_at_projection =
        (dotprodv / length * (x2_radius - x1_radius)) + x1_radius;

    const point proj_point = v_axis * dotprodv + x1_cone;

    const bool is_inside =
        (hg::distance(proj_point, p1) < radius_at_projection);
    // std::cout << " inside " << ((is_inside)?"TRUE":"FALSE") << "\n";
    return is_inside;
}

bool spatial_index::is_within(const point& p) const {
    // using value_type = decltype(p(0));  // local typedef not used

    box fake_box(p - point(0.01, 0.01, 0.01), p + point(0.01, 0.01, 0.01));

    hadoken::containers::small_vector<spatial_index_impl::indexed_box, 64> res;
    _pimpl->sp_index.query(bgi::contains(fake_box), std::back_inserter(res));

    if (res.empty()) {
        // do not match anything inside the R-tree
        return false;
    }

    // check if individual element intersect
    for (auto it = res.begin(); it < res.end(); ++it) {
        const int n_morpho = std::get<1>(*it);
        const int n_section = std::get<2>(*it);
        const int n_segment = std::get<3>(*it);
        const int indexed_type = std::get<4>(*it);

        const morpho_node& node = _pimpl->morphos[n_morpho]->get_node(n_section);

        switch (indexed_type) {
        case 0: {
            const neuron_soma& soma = static_cast<const neuron_soma&>(node);
            if (point_is_in_sphere(soma.get_sphere(), p)) {
                return true;
            }
            break;
        }

        case 1: {
            // return true;
            const neuron_section& my_section =
                static_cast<const neuron_section&>(node);
            if (point_is_in_truncated_cones(my_section.get_segment(n_segment),
                                            p)) {
                return true;
            }
            break;
        }

        case 2: {
            const neuron_section& my_section =
                static_cast<const neuron_section&>(node);
            if (point_is_in_sphere(my_section.get_junction(n_segment), p)) {
                return true;
            }
            break;
        }

        default: {
            // unmanaged case
            std::cerr << "Error: unmanaged indexed type, skip \n";
            return false;
        }
        }
    };

    return false;
}

} // morpho
