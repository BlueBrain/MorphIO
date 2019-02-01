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
} // namespace morphio
