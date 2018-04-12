#pragma once

#include <queue>
#include <stack>

#include <morphio/types.h>
namespace morphio
{
/**
An iterator class to iterate through sections;
 **/
template <typename T>
class Iterator
{
    friend class Section;
    T container;

public:
    Iterator(const Section& section);
    Iterator();
    bool operator==(Iterator other) const;
    bool operator!=(Iterator other) const;
    Section operator*() const;
    Iterator& operator++();
    Iterator operator++(int);
};
}
