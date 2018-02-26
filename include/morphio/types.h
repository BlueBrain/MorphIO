#pragma once

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include <stdint.h>

#include <morphio/enums.h>
#include <morphio/exceptions.h>
#include <morphio/vector_types.h>
#include <gsl/span>

#ifdef __GNUC__
#define BRAIN_UNUSED __attribute__((unused))
#else
#define BRAIN_UNUSED
#endif

// TODO: bcoste fix me
#include <iostream>
#define LBTHROW(x) (throw x)
#define LBERROR(x) (std::cerr << x << std::endl)
#define LBWARN std::cerr

/** @namespace morphio Blue Brain File IO classes */
namespace morphio
{
typedef std::string URI;

using namespace enums;
class Morphology;
class MorphologyInitData;
class Section;
class Soma;

namespace Property
{
struct Properties;
}

namespace detail
{
class NeurolucidaParser;
}
namespace mut
{
class Section;
class Soma;
class Morphology;
}

template <typename T>
class Iterator;
typedef std::pair<size_t, size_t> SectionRange;

typedef Iterator<std::stack<Section>> depth_iterator;
typedef Iterator<std::queue<Section>> breadth_iterator;
typedef Iterator<std::vector<Section>> upstream_iterator;

template <typename T> using range = gsl::span<T>;
}

// TODO: compile
#if 0
// if you have a type T in namespace N, the operator << for T needs to be in
// namespace N too
namespace boost
{
template <typename T>
inline std::ostream& operator<<(std::ostream& os,
                                const boost::multi_array<T, 2>& data)
{
    for (size_t i = 0; i < data.shape()[0]; ++i)
    {
        for (size_t j = 0; j < data.shape()[1]; ++j)
            os << data[i][j] << " ";
        os << std::endl;
    }
    return os;
}
}

namespace std
{
template <class T, class U>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& pair)
{
    return os << "[ " << pair.first << ", " << pair.second << " ]";
}
}
#endif
