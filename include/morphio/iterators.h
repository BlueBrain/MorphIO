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

public:
    Iterator();
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


} // namespace morphio
