#pragma once

#include <memory> // std::shared_ptr
#include <queue> // std::queue
#include <stack> // std::stack
#include <string> // std::string
#include <vector> // std::vector

#include <gsl/gsl>
#include <morphio/enums.h>
#include <morphio/exceptions.h>
#include <morphio/vector_types.h>


#define LBTHROW(x) (throw x)

/** @namespace morphio Blue Brain File IO classes */
namespace morphio {
using URI = std::string;

using namespace enums;
class Morphology;
template <class T>
class SectionBase;
class Section;
class MitoSection;
class Mitochondria;
class Soma;

namespace Property {
struct Properties;
}

namespace vasculature {
class Section;
class Vasculature;
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

using mito_depth_iterator = MitoIterator<std::stack<std::shared_ptr<MitoSection>>>;
using mito_breadth_iterator = MitoIterator<std::queue<std::shared_ptr<MitoSection>>>;
using mito_upstream_iterator = MitoIterator<std::vector<std::shared_ptr<MitoSection>>>;

template <typename T>
class Iterator;

using depth_iterator = Iterator<std::stack<std::shared_ptr<Section>>>;
using breadth_iterator = Iterator<std::queue<std::queue<std::shared_ptr<Section>>>>;
using upstream_iterator = Iterator<std::vector<std::shared_ptr<Section>>>;
} // namespace mut

using SectionRange = std::pair<size_t, size_t>;

template <typename T>
using range = gsl::span<T>;

} // namespace morphio
