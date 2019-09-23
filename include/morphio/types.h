#pragma once

#include <memory> // std::shared_ptr
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

namespace readers {
struct DebugInfo;
class ErrorMessages;
} // namespace readers

namespace mut {
class Section;
class MitoSection;
class Soma;
class Morphology;
class Mitochondria;
} // namespace mut

using SectionRange = std::pair<size_t, size_t>;

template <typename T>
using range = gsl::span<T>;

} // namespace morphio
