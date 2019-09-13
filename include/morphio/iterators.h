#pragma once

#include <queue>
#include <set>
#include <stack>
#include <iterator> // std::forward_iterator_tag

#include <morphio/types.h>

namespace morphio {
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class Iterator
{
    friend class Section;
    friend class Morphology;

    T container{};

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename T::value_type;
    using pointer = value_type*;
    using reference = value_type&;

    Iterator() = default;
    Iterator(const Section& section);
    Iterator(const Morphology& morphology);
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
    Section operator*() const;
    Iterator& operator++();
    Iterator operator++(int);
};


// Declare the specializations
template <>
breadth_iterator::Iterator(const Section&);
template <>
upstream_iterator::Iterator(const Section&);
template <>
breadth_iterator::Iterator(const Morphology&);
template <>
upstream_iterator::Iterator(const Morphology&);
template <>
Section depth_iterator::operator*() const;
template <>
Section breadth_iterator::operator*() const;
template <>
Section upstream_iterator::operator*() const;
template <>
depth_iterator& depth_iterator::operator++();
template <>
breadth_iterator& breadth_iterator::operator++();
template <>
upstream_iterator& upstream_iterator::operator++();

extern template class Iterator<std::stack<Section>>;
extern template class Iterator<std::queue<std::queue<Section>>>;
extern template class Iterator<std::vector<Section>>;

namespace vasculature {
class graph_iterator
{
    using iterator_category = std::forward_iterator_tag;
    using value_type = Section;
    using pointer = value_type*;
    using reference = value_type&;

    friend class Section;
    friend class Vasculature;

    std::set<Section> visited;
    std::stack<Section> container;

    graph_iterator() = default;

public:
    graph_iterator(const Section& vasculatureSection);
    graph_iterator(const Vasculature& vasculatureMorphology);
    bool operator==(const graph_iterator& other) const;
    bool operator!=(const graph_iterator& other) const;
    Section operator*() const;
    graph_iterator& operator++();
    graph_iterator operator++(int);
};
}  // namespace vasculature
}  // namespace morphio
