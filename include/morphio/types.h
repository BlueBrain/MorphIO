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

} // namespace morphio
