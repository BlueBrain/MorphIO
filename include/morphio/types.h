#pragma once

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include <stdint.h>

#include <gsl/gsl>
#include <morphio/enums.h>
#include <morphio/exceptions.h>
#include <morphio/vector_types.h>

#ifdef __GNUC__
#define BRAIN_UNUSED __attribute__((unused))
#else
#define BRAIN_UNUSED
#endif

// TODO: bcoste fix me
#include <iostream>
#define LBTHROW(x) (throw x)
#define LBWARN std::cerr

/** @namespace morphio Blue Brain File IO classes */
namespace morphio {
extern const std::string VERSION;
using URI = std::string;

using namespace enums;
class Morphology;
class MorphologyInitData;
template <class T>
class SectionBase;
class Section;
class MitoSection;
class Mitochondria;
class Soma;

namespace Property {
struct Properties;
}

namespace detail {
class NeurolucidaParser;
}

namespace plugin {
struct DebugInfo;
class ErrorMessages;
} // namespace plugin
namespace mut {
class Section;
class MitoSection;
class Soma;
class Morphology;
class Mitochondria;

template <typename T>
class MitoIterator;

template <typename T>
class Iterator;

using mito_depth_iterator = MitoIterator<std::stack<std::shared_ptr<MitoSection>>>;
using mito_breadth_iterator = MitoIterator<std::queue<std::shared_ptr<MitoSection>>>;
using mito_upstream_iterator = MitoIterator<std::vector<std::shared_ptr<MitoSection>>>;

using depth_iterator = Iterator<std::stack<std::shared_ptr<Section>>>;
using breadth_iterator = Iterator<std::queue<std::queue<std::shared_ptr<Section>>>>;
using upstream_iterator = Iterator<std::vector<std::shared_ptr<Section>>>;
} // namespace mut

template <typename T>
class MitoIterator;

template <typename T>
class Iterator;

using SectionRange = std::pair<size_t, size_t>;

using mito_depth_iterator = MitoIterator<std::stack<MitoSection>>;
using mito_breadth_iterator = MitoIterator<std::queue<MitoSection>>;
using mito_upstream_iterator = MitoIterator<std::vector<MitoSection>>;

using depth_iterator = Iterator<std::stack<Section>>;
using breadth_iterator = Iterator<std::queue<std::queue<Section>>>;
using upstream_iterator = Iterator<std::vector<Section>>;

template <typename T>
using range = gsl::span<T>;

std::ostream& operator<<(std::ostream& os, const std::vector<Point>&);
} // namespace morphio
