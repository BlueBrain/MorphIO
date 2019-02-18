/** @file enums.h Enumerations used in Brion. */
#pragma once

#include <ostream>

namespace morphio {
namespace enums {
/** The list of modifier flags that can be passed when loading a morphology
 See morphio::mut::modifiers for more informations **/
enum Option
{
    NO_MODIFIER = 0x00,
    TWO_POINTS_SECTIONS = 0x01,
    SOMA_SPHERE = 0x02,
    NO_DUPLICATES = 0x04,
    NRN_ORDER = 0x08
};

/**
   This enum should be kept in sync with the warnings
   defined in ErrorMessages.
   It is used to define which warnings are ignored by the user
**/
enum Warning
{
    UNDEFINED,
    MITOCHONDRIA_WRITE_NOT_SUPPORTED,
    WRITE_NO_SOMA,
    SOMA_NON_CONFORM,
    NO_SOMA_FOUND,
    DISCONNECTED_NEURITE,
    WRONG_DUPLICATE,
    APPENDING_EMPTY_SECTION,
    WRONG_ROOT_POINT,
    ONLY_CHILD,
};

/** The supported versions for morphology files. */
enum MorphologyVersion
{
    MORPHOLOGY_VERSION_H5_1 = 1,
    MORPHOLOGY_VERSION_H5_2 = 2,
    MORPHOLOGY_VERSION_H5_1_1 = 3,
    MORPHOLOGY_VERSION_ASC_1 = 4,
    MORPHOLOGY_VERSION_SWC_1 = 101,
    MORPHOLOGY_VERSION_UNDEFINED
};

enum AnnotationType
{
    SINGLE_CHILD,
};

/** The cell family represented by morphio::Morphology. */
enum CellFamily
{
    FAMILY_NEURON = 0,
    FAMILY_GLIA = 1
};

/** Output stream formatter for MorphologyVersion */
inline std::ostream& operator<<(std::ostream& os, const MorphologyVersion v)
{
    switch (v) {
    case MORPHOLOGY_VERSION_H5_1:
        return os << "h5v1";
    case MORPHOLOGY_VERSION_H5_2:
        return os << "h5v2";
    case MORPHOLOGY_VERSION_SWC_1:
        return os << "swcv1";
    default:
    case MORPHOLOGY_VERSION_UNDEFINED:
        return os << "UNDEFINED";
    }
}

enum IterType
{
    DEPTH_FIRST,
    BREADTH_FIRST,
    UPSTREAM
};

enum SomaType
{
    SOMA_UNDEFINED = 0,
    SOMA_SINGLE_POINT,
    SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS,
    SOMA_CYLINDERS,
    SOMA_SIMPLE_CONTOUR
};

/** Output stream formatter for MorphologyVersion */
inline std::ostream& operator<<(std::ostream& os, const SomaType v)
{
    switch (v) {
    case SOMA_SINGLE_POINT:
        return os << "SOMA_SINGLE_POINT";
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS:
        return os << "SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS";
    case SOMA_CYLINDERS:
        return os << "SOMA_CYLINDERS";
    case SOMA_SIMPLE_CONTOUR:
        return os << "SOMA_SIMPLE_CONTOUR";

    default:
    case SOMA_UNDEFINED:
        return os << "SOMA_UNDEFINED";
    }
}

/** Classification of neuron substructures. */
enum SectionType
{
    SECTION_UNDEFINED = 0,
    SECTION_SOMA = 1, //!< neuron cell body
    SECTION_AXON = 2,
    SECTION_DENDRITE = 3,        //!< general or basal dendrite (near to soma)
    SECTION_APICAL_DENDRITE = 4, //!< apical dendrite (far from soma)
    SECTION_GLIA_PROCESS = 2,    // TODO: nasty overload there
    SECTION_GLIA_ENDFOOT = 3,

    // All section types equal or above this number are custom types according
    // to neuromorpho.org standard
    SECTION_CUSTOM_START = 5,

    // CNIC defined types
    // It defines too extra types SWC_SECTION_FORK_POINT and
    // SWC_SECTION_END_POINT Consequently the custom types start at a higher
    // number See here:
    // http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
    SWC_SECTION_FORK_POINT = 5,
    SWC_SECTION_END_POINT = 6,
    SECTION_CNIC_CUSTOM_START = 7,

    SECTION_ALL = 32
};

/**
 * Specify the access mode of data.
 * @version 1.4
 */
enum AccessMode
{
    MODE_READ = 0x00000001,
    MODE_WRITE = 0x00000002,
    MODE_OVERWRITE = 0x00000004 | MODE_WRITE,
    MODE_READWRITE = MODE_READ | MODE_WRITE,
    MODE_READOVERWRITE = MODE_READ | MODE_OVERWRITE
};
} // namespace enums
} // namespace morphio
