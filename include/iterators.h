#pragma once

#include <stack>
#include <queue>

namespace minimorph {

/**
An iterator class to iterate through sections;
 **/
template <typename T> class Iterator {
    friend class Section;
    T container;
    Iterator() {}
public:
    explicit Iterator(const Section& section) { container.push(section); }
    // Iterator operator++(int) {Iterator retval = *this; ++(*this); return retval;}
    bool operator==(Iterator other) const {return container == other.container;}
    bool operator!=(Iterator other) const {return !(*this == other);}
    Section operator*() const;

    Iterator& operator++(){
        const auto& section = *(*this);
        container.pop();
        for(const auto& child: section.getChildren())
            container.push(child);
        return *this;
    }

};
}
