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
} // namespace mut
} // namespace morphio
