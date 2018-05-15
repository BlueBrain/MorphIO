#pragma once

#include <queue>
#include <stack>

#include <morphio/types.h>

namespace morphio
{
namespace mut
{
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class MitoIterator
{
    friend class Mitochondria;

    const Mitochondria& _mitochondria;

    T container;
    MitoIterator();

public:
    MitoIterator(const Mitochondria& mitochondria, uint32_t rootSectionId = -1);
    bool operator==(MitoIterator other) const;
    bool operator!=(MitoIterator other) const;
    uint32_t operator*() const;
    MitoIterator& operator++();
    MitoIterator operator++(int);
};
} // namespace mut
} // namespace morphio
