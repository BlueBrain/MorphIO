#include <morphio/iterators.h>
#include <morphio/section.h>

namespace morphio
{
class Section;

template <typename T>
Iterator<T>::Iterator(const Section& section)
{
    container.push(section);
}

template <typename T>
bool Iterator<T>::operator==(Iterator other) const
{
    return container == other.container;
}

template <typename T>
bool Iterator<T>::operator!=(Iterator other) const
{
    return !(*this == other);
}

template <typename T>
Iterator<T>& Iterator<T>::operator++()
{
    const auto& section = *(*this);
    container.pop();
    for (const auto& child : section.children())
        container.push(child);
    return *this;
}

template <typename T>
Iterator<T> Iterator<T>::operator++(int)
{
    Iterator retval = *this;
    ++(*this);
    return retval;
}

template <typename T>
Iterator<T>::Iterator()
{
}

// Specializations
template <>
Section depth_iterator::operator*() const
{
    return container.top();
}
template <>
Section breadth_iterator::operator*() const
{
    return container.front();
}
template <>
Section upstream_iterator::operator*() const
{
    return container[0];
}

template <>
upstream_iterator::Iterator(const Section& section)
{
    container.push_back(section);
}

template <>
upstream_iterator& upstream_iterator::operator++()
{
    const auto& section = *(*this);
    if (section.isRoot())
        container.pop_back();
    else
        container[0] = section.parent();
    return *this;
}

// Instantiations
template class Iterator<std::stack<Section>>;
template class Iterator<std::queue<Section>>;
template class Iterator<std::vector<Section>>;
}
