#include <morphio/mut/iterators.h>
#include <morphio/mut/morphology.h>

namespace morphio {
namespace mut {
class Section;

template <typename T>
Iterator<T>::Iterator(std::shared_ptr<Section> rootSection)
{
    container.push(rootSection);
}

template <>
breadth_iterator::Iterator(std::shared_ptr<Section> section)
{
    std::queue<std::shared_ptr<Section>> q;
    q.push(section);
    container.push(q);
}

template <typename T>
Iterator<T>::Iterator(const Morphology& morphology)
{
    auto roots = morphology.rootSections();
    for (auto it = roots.rbegin(); it != roots.rend(); ++it)
        container.push(*it);
}

template <>
breadth_iterator::Iterator(const Morphology& morphology)
{
    for (auto root : morphology.rootSections()) {
        std::queue<std::shared_ptr<Section>> q;
        q.push(root);
        container.push(q);
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
Iterator<T>::Iterator()
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
    return container.front().front();
}
template <>
std::shared_ptr<Section> upstream_iterator::operator*() const
{
    return container[0];
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
template <>
upstream_iterator::Iterator(const Morphology& morphology)
{
} // unused

template <>
upstream_iterator::Iterator(std::shared_ptr<Section> section)
{
    container.push_back(section);
}

template <>
breadth_iterator& breadth_iterator::operator++()
{
    const auto& section = *(*this);
    container.front().pop();
    for (auto& child : section->children())
        container.front().push(child);
    if (container.front().empty())
        container.pop();

    return *this;
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
template class Iterator<std::queue<std::queue<std::shared_ptr<Section>>>>;
template class Iterator<std::vector<std::shared_ptr<Section>>>;

} // namespace mut
} // namespace morphio
