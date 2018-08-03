#include <morphio/mut/iterators.h>
#include <morphio/mut/morphology.h>

namespace morphio
{
namespace mut
{
class Section;

template <typename T>
Iterator<T>::Iterator(const Morphology& morphology, std::shared_ptr<Section> rootSection) :
    _morphology(morphology)
{
    container.push(rootSection);
}

template <typename T>
Iterator<T>::Iterator(const Morphology& morphology) :
    _morphology(morphology)
{
    auto roots = _morphology.rootSections();
    for(auto it = roots.rbegin(); it != roots.rend(); ++it)
        container.push(*it);
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
    auto& children = section->children();
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
std::shared_ptr<Section> depth_iterator::operator*() const
{
    return container.top();
}
template <>
std::shared_ptr<Section> breadth_iterator::operator*() const
{
    return container.front();
}
template <>
std::shared_ptr<Section> upstream_iterator::operator*() const
{
    return container[0];
}

template <>
upstream_iterator::Iterator(const Morphology& morphology) :
    _morphology(morphology) {} // Unused

template <>
upstream_iterator::Iterator(const Morphology& morphology, std::shared_ptr<Section> section) :
    _morphology(morphology)
{
    container.push_back(section);
}

template <>
upstream_iterator& upstream_iterator::operator++()
{
    const auto& section = *(*this);
    if (section->isRoot())
        container.pop_back();
    else
        container[0] = section->parent();
    return *this;
}

// Instantiations
template class Iterator<std::stack<std::shared_ptr<Section>>>;
template class Iterator<std::queue<std::shared_ptr<Section>>>;
template class Iterator<std::vector<std::shared_ptr<Section>>>;

} // namespace mut
} // namespace morphio
