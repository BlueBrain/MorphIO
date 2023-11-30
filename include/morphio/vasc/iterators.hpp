/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <iterator>
#include <set>
#include <stack>

namespace morphio {
namespace vasculature {

template <typename SectionT, typename VasculatureT>
class graph_iterator_t
{
    std::set<SectionT> visited;
    std::stack<SectionT> container;

  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = SectionT;
    using difference_type = std::ptrdiff_t;
    using pointer = SectionT*;
    using reference = SectionT&;
    graph_iterator_t() = default;
    inline explicit graph_iterator_t(const SectionT& vasculatureSection);
    inline explicit graph_iterator_t(const VasculatureT& vasculatureMorphology);

    inline bool operator==(const graph_iterator_t& other) const;
    inline bool operator!=(const graph_iterator_t& other) const;
    inline const SectionT& operator*() const;

    inline graph_iterator_t& operator++();
    inline graph_iterator_t operator++(int);
};

template <typename SectionT, typename VasculatureT>
inline graph_iterator_t<SectionT, VasculatureT>::graph_iterator_t(
    const SectionT& vasculatureSection) {
    container.push(vasculatureSection);
}

template <typename SectionT, typename VasculatureT>
inline graph_iterator_t<SectionT, VasculatureT>::graph_iterator_t(
    const VasculatureT& vasculatureMorphology) {
    const auto& sections = vasculatureMorphology.sections();
    for (std::size_t i = 0; i < sections.size(); ++i) {
        if (sections[i].predecessors().empty()) {
            container.push(sections[i]);
            visited.insert(sections[i]);
        }
    }
}

template <typename SectionT, typename VasculatureT>
inline bool graph_iterator_t<SectionT, VasculatureT>::operator==(
    const graph_iterator_t& other) const {
    return container == other.container;
}

template <typename SectionT, typename VasculatureT>
inline bool graph_iterator_t<SectionT, VasculatureT>::operator!=(
    const graph_iterator_t& other) const {
    return !(*this == other);
}

template <typename SectionT, typename VasculatureT>
inline const SectionT& graph_iterator_t<SectionT, VasculatureT>::operator*() const {
    return container.top();
}

template <typename SectionT, typename VasculatureT>
inline graph_iterator_t<SectionT, VasculatureT>&
graph_iterator_t<SectionT, VasculatureT>::operator++() {
    const auto& section = *(*this);
    container.pop();
    const auto& neighbors = section.neighbors();
    for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
        if (visited.find(*it) == visited.end()) {
            container.push(*it);
            visited.insert(*it);
        }
    }
    return *this;
}

template <typename SectionT, typename VasculatureT>
inline graph_iterator_t<SectionT, VasculatureT>
graph_iterator_t<SectionT, VasculatureT>::operator++(int) {
    graph_iterator_t retval = *this;
    ++(*this);
    return retval;
}

}  // namespace vasculature
}  // namespace morphio
