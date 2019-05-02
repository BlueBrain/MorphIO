#pragma once

#include <queue>
#include <stack>

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

// Declare the specializations
template <> breadth_iterator::Iterator(const Section&);
template <> depth_iterator::Iterator(const Section&);
template <> upstream_iterator::Iterator(const Section&);
template <> breadth_iterator::Iterator(const Morphology&);
template <> depth_iterator::Iterator(const Morphology&);
template <> upstream_iterator::Iterator(const Morphology&);
template <> Section depth_iterator::operator*() const;
template <> Section breadth_iterator::operator*() const;
template <> Section upstream_iterator::operator*() const;
template <> depth_iterator& depth_iterator::operator++();
template <> breadth_iterator& breadth_iterator::operator++();
template <> upstream_iterator& upstream_iterator::operator++();

// Explicit declaration
extern template class Iterator<std::stack<Section>>;
extern template class Iterator<std::queue<std::queue<Section>>>;
extern template class Iterator<std::vector<Section>>;

} // namespace morphio
