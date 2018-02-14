#pragma once

#include <iostream>
#include <vector>

template <typename T>
std::ostream& operator<<(std::ostream& s, const std::vector<T>& v)
{
    const size_t max = 5;
    for (size_t i = 0; i < v.size() && i < max; i++)
    {
        s << " " << v[i];
    }
    if (v.size() > max)
    {
        s << " ...";
    }
    return s;
}
