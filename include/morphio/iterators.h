#pragma once

#include <queue>
#include <set>
#include <stack>

#include <morphio/types.h>
#include <morphio/vasc/section.h>

namespace morphio {
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
