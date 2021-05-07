#pragma once

#include <memory>  // std::shared_ptr
#include <string>  // std::string
#include <vector>  // std::vector

#include <gsl/gsl>
#include <morphio/enums.h>
#include <morphio/exceptions.h>
#include <morphio/vector_types.h>


/** @namespace morphio Blue Brain File IO classes */
namespace morphio {

using namespace enums;
class EndoplasmicReticulum;
class MitoSection;
class Mitochondria;
class Morphology;
class Section;
template <class T>
class SectionBase;
class Soma;

namespace Property {
struct Properties;
}

namespace vasculature {
class Section;
class Vasculature;
}  // namespace vasculature

namespace readers {
struct DebugInfo;
class ErrorMessages;
}  // namespace readers

namespace mut {
class EndoplasmicReticulum;
class MitoSection;
class Mitochondria;
class Morphology;
class Section;
class Soma;
}  // namespace mut

using SectionRange = std::pair<size_t, size_t>;

// A tuple (file format (std::string), major version, minor version)
using MorphologyVersion = std::tuple<std::string, uint32_t, uint32_t>;

template <typename T>
using range = gsl::span<T>;

}  // namespace morphio
