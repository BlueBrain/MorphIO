#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/mitochondria.h>

namespace morphio {
namespace mut {
class MitoSection;

template <typename T>
MitoIterator<T>::MitoIterator(const Mitochondria& mitochondria,
    std::shared_ptr<MitoSection> rootSection)
    : _mitochondria(mitochondria)
{
    container.push(rootSection);
}

template <typename T>
MitoIterator<T>::MitoIterator(const Mitochondria& mitochondria)
    : _mitochondria(mitochondria)
{
    auto roots = _mitochondria.rootSections();
    for (auto it = roots.rbegin(); it != roots.rend(); ++it)
        container.push(*it);
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
MitoIterator<T>::MitoIterator()
    : _mitochondria(Mitochondria())
{
}

// Specializations
template <>
std::shared_ptr<MitoSection> mito_depth_iterator::operator*() const
{
    return container.top();
}
template <>
std::shared_ptr<MitoSection> mito_breadth_iterator::operator*() const
{
    return container.front();
}
template <>
std::shared_ptr<MitoSection> mito_upstream_iterator::operator*() const
{
    return container[0];
}

template <>
mito_upstream_iterator::MitoIterator(const Mitochondria& mitochondria)
    : _mitochondria(mitochondria)
{
}

template <>
mito_upstream_iterator::MitoIterator(const Mitochondria& mitochondria,
    std::shared_ptr<MitoSection> section)
    : _mitochondria(mitochondria)
{
    container.push_back(section);
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
template class MitoIterator<std::stack<std::shared_ptr<MitoSection>>>;
template class MitoIterator<std::queue<std::shared_ptr<MitoSection>>>;
template class MitoIterator<std::vector<std::shared_ptr<MitoSection>>>;

} // namespace mut
} // namespace morphio
