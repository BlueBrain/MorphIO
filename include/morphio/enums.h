#pragma once

#include <iosfwd>  // std::ostream

namespace morphio {
/** A collection of enums for different util purposes **/
namespace enums {

enum LogLevel { ERROR, WARNING, INFO, DEBUG };

/** The list of modifier flags that can be passed when loading a morphology
 * See morphio::mut::modifiers for more information **/
enum Option {
    NO_MODIFIER = 0x00,          //!< Read morphology as is without any modification
    TWO_POINTS_SECTIONS = 0x01,  //!< Read sections only with 2 or more points
    SOMA_SPHERE = 0x02,          //!< Interpret morphology soma as a sphere
    NO_DUPLICATES = 0x04,        //!< Skip duplicating points
    NRN_ORDER = 0x08             //!< Order of neurites will be the same as in NEURON simulator
};

/**
   All possible warnings that can be ignored by user. This enum should be kept in sync with the
   warnings defined in morphio::ErrorMessages.
**/
enum Warning {
    UNDEFINED,                         //!< undefined value
    MITOCHONDRIA_WRITE_NOT_SUPPORTED,  //!< Mitochondria can be saved in H5 format only
    WRITE_NO_SOMA,                     //!< Writing without a soma
    SOMA_NON_CONFORM,      //!< Soma does not conform the three point soma spec from NeuroMorpho.org
    NO_SOMA_FOUND,         //!< No soma found in a file
    DISCONNECTED_NEURITE,  //!< Found a disconnected neurite in a morphology
    WRONG_DUPLICATE,       //!< A wrong duplicate point in a section
    APPENDING_EMPTY_SECTION,  //!< Appending of an empty section
    WRONG_ROOT_POINT,         //!< A wrong root point of a neurite in the case of 3 points soma
    ONLY_CHILD,               //!< Single child sections are not allowed in SWC format
    WRITE_EMPTY_MORPHOLOGY,   //!< Writing empty morphology
    ZERO_DIAMETER             //!< Zero section diameter
};

enum AnnotationType {
    SINGLE_CHILD,
};

/** The cell family represented by morphio::Morphology. */
enum CellFamily {
    NEURON = 0,  //!< Neuron
    GLIA = 1,    //!< Glia
    SPINE = 2    //!< Spine
};

/** Soma type. */
enum SomaType {
    SOMA_UNDEFINED = 0,                      //!< Undefined soma
    SOMA_SINGLE_POINT,                       //!< Single point soma
    SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS,  //!< Soma made of three cylinders
    SOMA_CYLINDERS,                          //!< Soma made of cylinders
    SOMA_SIMPLE_CONTOUR                      //!< Contour soma. Assumed that contour is in XY plane
};
std::ostream& operator<<(std::ostream& os, SomaType v);

/** Classification of neuron substructures. */
enum SectionType {
    SECTION_UNDEFINED = 0,        //!< Undefined section
    SECTION_SOMA = 1,             //!< Neuron cell body
    SECTION_AXON = 2,             //!< Axon section
    SECTION_DENDRITE = 3,         //!< General or basal dendrite (near to soma)
    SECTION_APICAL_DENDRITE = 4,  //!< Apical dendrite (far from soma)

    SECTION_GLIA_PERIVASCULAR_PROCESS = 2,  // Note: overlaps with SECTION_AXON
    SECTION_GLIA_PROCESS = 3,               // Note: overlaps with SECTION_DENDRITE

    SECTION_SPINE_NECK = 2,  // Note: overlaps with SECTION_AXON
    SECTION_SPINE_HEAD = 3,  // Note: overlaps with SECTION_DENDRITE

    // unnamed custom section types
    SECTION_CUSTOM_5 = 5,    //!< Custom section type
    SECTION_CUSTOM_6 = 6,    //!< Custom section type
    SECTION_CUSTOM_7 = 7,    //!< Custom section type
    SECTION_CUSTOM_8 = 8,    //!< Custom section type
    SECTION_CUSTOM_9 = 9,    //!< Custom section type
    SECTION_CUSTOM_10 = 10,  //!< Custom section type
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

    SECTION_ALL = 32  //!< Any section type
};

/** Classification of vasculature section types. */
enum VascularSectionType {
    SECTION_NOT_DEFINED = 0,         //!< Undefined
    SECTION_VEIN = 1,                //!< Vein
    SECTION_ARTERY = 2,              //!< Artery
    SECTION_VENULE = 3,              //!< Venule
    SECTION_ARTERIOLE = 4,           //!< Arteriole
    SECTION_VENOUS_CAPILLARY = 5,    //!< Venous capillary
    SECTION_ARTERIAL_CAPILLARY = 6,  //!< Arterial capillary
    SECTION_TRANSITIONAL = 7,        //!< Transitional
    SECTION_CUSTOM = 8               //!< Custom section type
};

}  // namespace enums
}  // namespace morphio
