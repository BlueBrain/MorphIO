#include <morphio/mito_iterators.h>
#include <morphio/mito_section.h>

namespace morphio {
class MitoSection;

template <typename T>
MitoIterator<T>::MitoIterator(const MitoSection& section)
{
    container.push(section);
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
    for (const auto& child : section.children())
        container.push(child);
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
{
}

// Specializations
template <>
MitoSection mito_depth_iterator::operator*() const
{
    return container.top();
}
template <>
MitoSection mito_breadth_iterator::operator*() const
{
    return container.front();
}
template <>
MitoSection mito_upstream_iterator::operator*() const
{
    return container[0];
}

template <>
mito_upstream_iterator::MitoIterator(const MitoSection& section)
{
    container.push_back(section);
}

template <>
mito_upstream_iterator& mito_upstream_iterator::operator++()
{
    const auto& section = *(*this);
    if (section.isRoot())
        container.pop_back();
    else
        container[0] = section.parent();
    return *this;
}

// Instantiations
template class MitoIterator<std::stack<MitoSection>>;
template class MitoIterator<std::queue<MitoSection>>;
template class MitoIterator<std::vector<MitoSection>>;
} // namespace morphio
