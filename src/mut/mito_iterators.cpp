#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/mitochondria.h>

namespace morphio
{
namespace mut
{
class MitoSection;

template <typename T>
MitoIterator<T>::MitoIterator(const Mitochondria& mitochondria, int rootSectionId) :
    _mitochondria(mitochondria)
{
    if(rootSectionId != -1)
        container.push(rootSectionId);
    else {
        auto roots = _mitochondria.rootSections();
        for(auto it = roots.rbegin(); it != roots.rend(); ++it)
            container.push(*it);
    }
}

template <typename T>
bool MitoIterator<T>::operator==(MitoIterator other) const
{
    return container == other.container;
}

template <typename T>
bool MitoIterator<T>::operator!=(MitoIterator other) const
{
    return !(*this == other);
}

template <typename T>
MitoIterator<T>& MitoIterator<T>::operator++()
{
    const auto& section = *(*this);
    container.pop();
    auto& children = _mitochondria.children(section);
    for (auto it = children.rbegin(); it != children.rend(); ++it)
        container.push(*it);
    return *this;
}

template <typename T>
MitoIterator<T> MitoIterator<T>::operator++(int)
{
    MitoIterator retval = *this;
    ++(*this);
    return retval;
}

template <typename T>
MitoIterator<T>::MitoIterator() : _mitochondria(Mitochondria())
{
}

// Specializations
template <>
uint32_t mito_depth_iterator::operator*() const
{
    return container.top();
}
template <>
uint32_t mito_breadth_iterator::operator*() const
{
    return container.front();
}
template <>
uint32_t mito_upstream_iterator::operator*() const
{
    return container[0];
}

template <>
mito_upstream_iterator::MitoIterator(const Mitochondria& mitochondria, int sectionId) :
    _mitochondria(mitochondria)
{
    container.push_back(sectionId);
}

template <>
mito_upstream_iterator& mito_upstream_iterator::operator++()
{
    // const auto& section = *(*this);
    // if (section.isRoot())
    //     container.pop_back();
    // else
    //     container[0] = section.parent();
    return *this;
}

// Instantiations
template class MitoIterator<std::stack<uint32_t>>;
template class MitoIterator<std::queue<uint32_t>>;
template class MitoIterator<std::vector<uint32_t>>;

} // namespace mut
} // namespace morphio
