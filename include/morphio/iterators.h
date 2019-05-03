#pragma once

#include <queue>
#include <set>
#include <stack>

#include <morphio/types.h>
#include <morphio/vasc/section.h>

namespace morphio {
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class Iterator
{
    friend class Section;
    friend class Morphology;

    T container;

    Iterator();

public:
    Iterator(const Section& section);
    Iterator(const Morphology& morphology);
    bool operator==(Iterator other) const;
    bool operator!=(Iterator other) const;
    Section operator*() const;
    Iterator& operator++();
    Iterator operator++(int);
};


class graph_iterator
{
    friend class vasculature::Section;
    friend class vasculature::Vasculature;
    std::set<vasculature::Section> visited;

    std::stack<vasculature::Section> container;

        graph_iterator();


public:
    graph_iterator(const vasculature::Section& vasculatureSection);
    graph_iterator(const vasculature::Vasculature& vasculatureMorphology);
    bool operator==(graph_iterator other) const;
    bool operator!=(graph_iterator other) const;
    vasculature::Section operator*() const;
    graph_iterator& operator++();
    graph_iterator operator++(int);
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


} // namespace morphio
