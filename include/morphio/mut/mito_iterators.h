#pragma once

#include <queue>
#include <stack>

#include <morphio/mut/mitochondria.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class MitoIterator
{
    friend class Mitochondria;

    const Mitochondria _mitochondria;

    T container;
    MitoIterator();

public:
    MitoIterator(const Mitochondria& mitochondria,
        std::shared_ptr<MitoSection> rootSectionId = nullptr);
    MitoIterator(const Mitochondria& mitochondria);
    bool operator==(MitoIterator other) const;
    bool operator!=(MitoIterator other) const;
    std::shared_ptr<MitoSection> operator*() const;
    MitoIterator& operator++();
    MitoIterator operator++(int);
};

// Declare the specializations
template <>
std::shared_ptr<MitoSection> MitoIterator<std::stack<std::shared_ptr<MitoSection>>>::operator*() const;
template <>
std::shared_ptr<MitoSection> MitoIterator<std::queue<std::shared_ptr<MitoSection>>>::operator*() const;
template <>
std::shared_ptr<MitoSection> MitoIterator<std::vector<std::shared_ptr<MitoSection>>>::operator*() const;
template <>
mito_upstream_iterator::MitoIterator(const Mitochondria&);
template <>
mito_upstream_iterator::MitoIterator(const Mitochondria&, std::shared_ptr<MitoSection>);
template <>
mito_upstream_iterator& mito_upstream_iterator::operator++();

// Explicit declaration
extern template class MitoIterator<std::stack<std::shared_ptr<MitoSection>>>;
extern template class MitoIterator<std::queue<std::shared_ptr<MitoSection>>>;
extern template class MitoIterator<std::vector<std::shared_ptr<MitoSection>>>;

} // namespace mut
} // namespace morphio
