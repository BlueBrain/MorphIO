#pragma once

#include <utility>  // std::pair

#include <morphio/enums.h>
#include <morphio/exceptions.h>
#include <morphio/vector_types.h>

namespace morphio {

using namespace enums;
class DendriticSpine;
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
}  // namespace Property

/** Functionality for vasculature (blood)  **/
namespace vasculature {
class Section;
class Vasculature;
}  // namespace vasculature

/** Functionality for reading of morphologies from files  **/
namespace readers {
struct DebugInfo;
class ErrorMessages;
}  // namespace readers

/** Functionality for mutating(editing) of morphologies  **/
namespace mut {
class DendriticSpine;
class EndoplasmicReticulum;
class MitoSection;
class Mitochondria;
class Morphology;
class Section;
class Soma;
}  // namespace mut

using SectionRange = std::pair<size_t, size_t>;

}  // namespace morphio
