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
class Iterator
{
    friend class Morphology;

    const Morphology& _morphology;

    T container;
    Iterator();

public:
    Iterator(const Morphology& morphology, std::shared_ptr<Section> rootSection);
    Iterator(const Morphology& morphology);
    bool operator==(Iterator other) const;
    bool operator!=(Iterator other) const;
    std::shared_ptr<Section> operator*() const;
    Iterator& operator++();
    Iterator operator++(int);
};
} // namespace mut
} // namespace morphio
