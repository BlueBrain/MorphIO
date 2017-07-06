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
 *
 */

#include <morpho/morpho_stats.hpp>
#include <numeric>

namespace morpho {

namespace stats {

std::size_t total_number_sectiones(const morpho_tree& tree) {
    return tree.get_tree_size();
}

std::size_t total_number_point(const morpho_tree& tree) {

    std::vector<morpho_node const*> all_nodes = tree.get_all_nodes();
    std::size_t res = 0;

    std::for_each(
        all_nodes.begin(), all_nodes.end(), [&](morpho_node const* node_ref) {

            if (node_ref->is_of_type(morpho_node_type::neuron_soma_type)) {
                const neuron_soma& soma =
                    static_cast<const neuron_soma&>(*node_ref);
                res += soma.get_line_loop().size();
            } else if (node_ref->is_of_type(
                           morpho_node_type::neuron_section_type)) {
                const neuron_section& section =
                    static_cast<const neuron_section&>(*node_ref);
                res += section.get_number_points();
            }

        });
    return res;
}

template <typename Fun, typename T>
T for_all_radius_reduction(const morpho_tree& tree, const Fun& f, T val) {

    const T max_radius = val;
    double res = max_radius;

    std::vector<morpho_node const*> all_nodes = tree.get_all_nodes();

    std::for_each(
        all_nodes.begin(), all_nodes.end(), [&](morpho_node const* node) {

            if (node->is_of_type(morpho_node_type::neuron_section_type)) {
                const neuron_section& section =
                    static_cast<const neuron_section&>(*node);
                const std::vector<double>& all_radius = section.get_radius();

                res = std::accumulate(
                    all_radius.begin(), all_radius.end(), res,
                    [&](double a, double b) { return f(a, b); });
            }

        });
    return res;
}

double min_radius_segment(const morpho_tree& tree) {
    return for_all_radius_reduction(
        tree, [](double a, double b) { return std::min(a, b); },
        std::numeric_limits<double>::max());
}

double max_radius_segment(const morpho_tree& tree) {
    return for_all_radius_reduction(
        tree, [](double a, double b) { return std::max(a, b); },
        std::numeric_limits<double>::min());
}

double median_radius_segment(const morpho_tree& tree) {
    std::vector<morpho_node const*> all_nodes = tree.get_all_nodes();

    std::vector<double> all_radius;
    all_radius.reserve(total_number_point(tree));

    std::for_each(
        all_nodes.begin(), all_nodes.end(), [&](morpho_node const* node) {

            if (node->is_of_type(morpho_node_type::neuron_section_type)) {
                const neuron_section& section =
                    static_cast<const neuron_section&>(*node);
                const std::vector<double>& all_radius_section =
                    section.get_radius();
                std::copy(all_radius_section.begin(), all_radius_section.end(),
                          std::back_inserter(all_radius));
            }

        });

    if (all_radius.size() < 1) {
        throw std::invalid_argument(
            "morpho_tree should contains at least one point");
    }

    std::sort(all_radius.begin(), all_radius.end());
    return all_radius[all_radius.size() / 2];
}

bool has_duplicated_points(const morpho_tree& tree) {
    std::vector<int> index_node(tree.get_tree_size());
    std::iota(index_node.begin(), index_node.end(), 0);

    return std::any_of(index_node.begin(), index_node.end(), [&](int i) {
        const morpho_node& node = tree.get_node(i);
        const int parent_id = tree.get_parent(i);
        point last_point(std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());

        if (parent_id >= 0) { // get the parent point if it exists
            const morpho_node& parent_node = tree.get_node(parent_id);
            if (node.is_of_type(morpho_node_type::neuron_section_type)) {
                const neuron_section& parent_b =
                    static_cast<const neuron_section&>(parent_node);
                const std::size_t size_section = parent_b.get_number_points();
                last_point = parent_b.get_points()[size_section - 1];
            }
        }

        if (node.is_of_type(morpho_node_type::neuron_section_type)) {
            const neuron_section& b = static_cast<const neuron_section&>(node);
            for (const auto& point : b.get_points()) {
                if (last_point.close_to(point))
                    return true;
                last_point = point;
            }
        }
        return false;
    });
}

} // stats

} // morpho
