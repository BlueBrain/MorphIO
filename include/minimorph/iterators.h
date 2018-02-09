#pragma once

#include <stack>
#include <queue>

#include <minimorph/types.h>
namespace minimorph {



/**
An iterator class to iterate through sections;
 **/
template <typename T> class Iterator {
    friend class Section;
    T container;
    Iterator() {}
public:
    explicit Iterator(const Section& section);
    bool operator==(Iterator other) const;
    bool operator!=(Iterator other) const;
    Section operator*() const;
    Iterator& operator++();
    Iterator operator++(int);

};
}
