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
template <class T> class SectionBase;
class Section;
class MitoSection;
class Mitochondria;
class Soma;

namespace Property
{
struct Properties;
}

namespace detail
{
class NeurolucidaParser;
}

namespace plugin
{
    class DebugInfo;
    class ErrorMessages;
}
namespace mut
{
class Section;
class MitoSection;
class Soma;
class Morphology;
class Mitochondria;

template <typename T>
class MitoIterator;

template <typename T>
class Iterator;

typedef MitoIterator<std::stack<uint32_t>> mito_depth_iterator;
typedef MitoIterator<std::queue<uint32_t>> mito_breadth_iterator;
typedef MitoIterator<std::vector<uint32_t>> mito_upstream_iterator;

typedef Iterator<std::stack<std::shared_ptr<Section>>> depth_iterator;
typedef Iterator<std::queue<std::shared_ptr<Section>>> breadth_iterator;
typedef Iterator<std::vector<std::shared_ptr<Section>>> upstream_iterator;
}

template <typename T>
class MitoIterator;

template <typename T>
class Iterator;


typedef std::pair<size_t, size_t> SectionRange;

typedef MitoIterator<std::stack<MitoSection>> mito_depth_iterator;
typedef MitoIterator<std::queue<MitoSection>> mito_breadth_iterator;
typedef MitoIterator<std::vector<MitoSection>> mito_upstream_iterator;

typedef Iterator<std::stack<Section>> depth_iterator;
typedef Iterator<std::queue<Section>> breadth_iterator;
typedef Iterator<std::vector<Section>> upstream_iterator;

template <typename T> using range = gsl::span<T>;

std::ostream& operator<<(std::ostream& os, const std::vector<Point>&);
} // namespace morphio
