#include <morphio/mut/iterators.h>
#include <morphio/mut/morphology.h>

namespace morphio
{
namespace mut
{
class Section;

template <typename T>
Iterator<T>::Iterator(const Morphology& morphology, uint32_t rootSectionId) :
    _morphology(morphology)
{
    if(rootSectionId != -1)
        container.push(rootSectionId);
    else {
        auto roots = _morphology.rootSections();
        for(auto it = roots.rbegin(); it != roots.rend(); ++it)
            container.push(*it);
    }
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
    auto& children = _morphology.children(section);
    for (auto it = children.rbegin(); it != children.rend(); ++it)
        container.push(*it);
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
Iterator<T>::Iterator() : _morphology(Morphology())
{
}

// Specializations
template <>
uint32_t depth_iterator::operator*() const
{
    return container.top();
}
template <>
uint32_t breadth_iterator::operator*() const
{
    return container.front();
}
template <>
uint32_t upstream_iterator::operator*() const
{
    return container[0];
}

template <>
upstream_iterator::Iterator(const Morphology& morphology, uint32_t sectionId) :
    _morphology(morphology)
{
    container.push_back(sectionId);
}

template <>
upstream_iterator& upstream_iterator::operator++()
{
    // const auto& section = *(*this);
    // if (section.isRoot())
    //     container.pop_back();
    // else
    //     container[0] = section.parent();
    return *this;
}

// Instantiations
template class Iterator<std::stack<uint32_t>>;
template class Iterator<std::queue<uint32_t>>;
template class Iterator<std::vector<uint32_t>>;

} // namespace mut
} // namespace morphio
