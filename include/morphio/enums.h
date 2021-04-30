#pragma once
#include <iosfwd>

namespace morphio {
namespace enums {

enum LogLevel { ERROR, WARNING, INFO, DEBUG };

/** The list of modifier flags that can be passed when loading a morphology
 See morphio::mut::modifiers for more information **/
enum Option {
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
enum Warning {
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
    WRITE_EMPTY_MORPHOLOGY,
    ZERO_DIAMETER
};

enum AnnotationType {
    SINGLE_CHILD,
};

/** The cell family represented by morphio::Morphology. */
enum CellFamily { NEURON = 0, GLIA = 1 };

enum SomaType {
    SOMA_UNDEFINED = 0,
    SOMA_SINGLE_POINT,
    SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS,
    SOMA_CYLINDERS,
    SOMA_SIMPLE_CONTOUR
};
std::ostream& operator<<(std::ostream& os, SomaType v);

/** Classification of neuron substructures. */
enum SectionType {
    SECTION_UNDEFINED = 0,
    SECTION_SOMA = 1,  //!< neuron cell body
    SECTION_AXON = 2,
    SECTION_DENDRITE = 3,         //!< general or basal dendrite (near to soma)
    SECTION_APICAL_DENDRITE = 4,  //!< apical dendrite (far from soma)

    SECTION_GLIA_PERIVASCULAR_PROCESS = 2,
    SECTION_GLIA_PROCESS = 3,  // TODO: nasty overload there

    // unnamed custom section types
    SECTION_CUSTOM_5 = 5,
    SECTION_CUSTOM_6 = 6,
    SECTION_CUSTOM_7 = 7,
    SECTION_CUSTOM_8 = 8,
    SECTION_CUSTOM_9 = 9,
    SECTION_CUSTOM_10 = 10,
    // All section types equal or above this number are invalid custom types according
    // to neuromorpho.org standard (http://neuromorpho.org/StdSwc1.21.jsp)
    SECTION_OUT_OF_RANGE_START = 11,

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

enum VascularSectionType {
    SECTION_NOT_DEFINED = 0,
    SECTION_VEIN = 1,
    SECTION_ARTERY = 2,
    SECTION_VENULE = 3,
    SECTION_ARTERIOLE = 4,
    SECTION_VENOUS_CAPILLARY = 5,
    SECTION_ARTERIAL_CAPILLARY = 6,
    SECTION_TRANSITIONAL = 7,
    SECTION_CUSTOM = 8
};

/**
 * Specify the access mode of data.
 * @version 1.4
 */
enum AccessMode {
    MODE_READ = 0x00000001,
    MODE_WRITE = 0x00000002,
    MODE_OVERWRITE = 0x00000004 | MODE_WRITE,
    MODE_READWRITE = MODE_READ | MODE_WRITE,
    MODE_READOVERWRITE = MODE_READ | MODE_OVERWRITE
};

}  // namespace enums
}  // namespace morphio
