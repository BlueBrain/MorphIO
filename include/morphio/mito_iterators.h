#pragma once

#include <queue>
#include <stack>

#include <morphio/types.h>
namespace morphio {
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class MitoIterator
{
    friend class MitoSection;
    T container;

public:
    MitoIterator(const MitoSection& section);
    MitoIterator();
    bool operator==(MitoIterator other) const;
    bool operator!=(MitoIterator other) const;
    MitoSection operator*() const;
    MitoIterator& operator++();
    MitoIterator operator++(int);
};

// Declare the specializations
template <>
MitoSection MitoIterator<std::stack<MitoSection>>::operator*() const;
template <>
MitoSection MitoIterator<std::queue<MitoSection>>::operator*() const;
template <>
MitoSection MitoIterator<std::vector<MitoSection>>::operator*() const;
template <>
mito_upstream_iterator::MitoIterator(const MitoSection& section);
template <>
mito_upstream_iterator& mito_upstream_iterator::operator++();

// Explicit declaration
extern template class MitoIterator<std::stack<MitoSection>>;
extern template class MitoIterator<std::queue<MitoSection>>;
extern template class MitoIterator<std::vector<MitoSection>>;

} // namespace morphio
