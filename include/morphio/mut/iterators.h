#pragma once

#include <queue>
#include <stack>

#include <morphio/types.h>
namespace morphio {
namespace mut {
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class Iterator
{
    friend class Morphology;

    T container;

public:
    Iterator(std::shared_ptr<Section> rootSection);
    Iterator(const Morphology& morphology);
    Iterator();
    bool operator==(Iterator other) const;
    bool operator!=(Iterator other) const;
    std::shared_ptr<Section> operator*() const;
    Iterator& operator++();
    Iterator operator++(int);
};

// Declare the specializations
template <>
breadth_iterator::Iterator(const std::shared_ptr<Section>);
template <>
upstream_iterator::Iterator(const std::shared_ptr<Section>);
template <>
breadth_iterator::Iterator(const Morphology&);
template <>
upstream_iterator::Iterator(const Morphology&);
template <>
std::shared_ptr<Section> depth_iterator::operator*() const;
template <>
std::shared_ptr<Section> breadth_iterator::operator*() const;
template <>
std::shared_ptr<Section> upstream_iterator::operator*() const;
template <>
breadth_iterator& breadth_iterator::operator++();
template <>
upstream_iterator& upstream_iterator::operator++();

// Explicit declaration
extern template class Iterator<std::stack<std::shared_ptr<Section>>>;
extern template class Iterator<std::queue<std::queue<std::shared_ptr<Section>>>>;
extern template class Iterator<std::vector<std::shared_ptr<Section>>>;


} // namespace mut
} // namespace morphio
