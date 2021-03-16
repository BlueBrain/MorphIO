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
template <class Family>
class Node;

template <class TSection, class CRTP, class Mut>
class TTree;
class Morphology;
class GlialCell;

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
class GlialCell;
class Section;
class GlialSection;
class Soma;
}  // namespace mut

using SectionRange = std::pair<size_t, size_t>;
using MorphologyVersion = std::tuple<std::string, uint32_t, uint32_t>;

template <typename T>
using range = gsl::span<T>;


struct CellFamily {
    struct NEURON {
        using  Type = SectionType;
        static constexpr int value = 0;
    };
    struct GLIA {
        using  Type = GlialSectionType;
        static constexpr int value = 1;
    };
};

using NeuronalSection = Node<CellFamily::NEURON>;
using GlialSection = Node<CellFamily::GLIA>;

// legacy name
using Section = NeuronalSection;

}  // namespace morphio
