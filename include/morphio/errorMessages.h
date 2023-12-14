/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <map>        // std::map
#include <memory>     // std::shared_ptr
#include <string>     // std::string
#include <vector>     // std::vector

#include <morphio/enums.h>        // Warning, Option
#include <morphio/error_warning_handling.h> // ErrorAndWarningHandler
#include <morphio/mut/section.h>  // Warning, Option

namespace morphio {

/** Set the maximum number of warnings to be printed on screen **/
void set_maximum_warnings(int n_warnings);
/** Set whether to interpet warning as errors **/
void set_raise_warnings(bool is_raise);
/** Set a warning to ignore **/
void set_ignored_warning(Warning warning, bool ignore = true);
/** Set an array of warnings to ignore **/
void set_ignored_warning(const std::vector<Warning>& warning, bool ignore = true);
/** Print a warning. Raises an error if  `set_raise_warnings` was set to `true`. **/
void printError(Warning warning, const std::string& msg);

namespace readers {

/** Level of error reporting **/
enum ErrorLevel {
    INFO,     //!< Info
    WARNING,  //!< Warning
    ERROR     //!< Error
};

/** Class that can generate error messages and holds a collection of predefined errors
    messages **/
class ErrorMessages
{
  public:
    ErrorMessages() = default;

    /** Constructor.

       \param uri path to a morphology file.
     */
    explicit ErrorMessages(const std::string& uri)
        : _uri(uri) {}

    /** Is the output of the warning ignored */
    static bool isIgnored(const Warning& warning);

    /** Generate an error message. */
    std::string errorMsg(long unsigned int lineNumber,
                         ErrorLevel errorLevel,
                         const std::string& msg = std::string()) const;

    ////////////////////////////////////////////////////////////////////////////////
    //              ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    /** Non parsable line error message */
    std::string ERROR_LINE_NON_PARSABLE(long unsigned int lineNumber) const;

    /** Unsupported morphology section type error message */
    std::string ERROR_UNSUPPORTED_SECTION_TYPE(long unsigned int lineNumber,
                                               const SectionType& type) const;

    /** Unsupported vasculature section type error message */
    std::string ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(long unsigned int lineNumber,
                                                           const VascularSectionType& type) const;

    /** Multiple somas error message */
    std::string ERROR_MULTIPLE_SOMATA(const std::vector<unsigned int>& lineNumbers) const;

    /** Missing section parent error message */
    std::string ERROR_MISSING_PARENT(unsigned int id,
                                     int parentId,
                                     unsigned int lineNumber) const;

    /** Bifurcating soma error message */
    std::string ERROR_SOMA_BIFURCATION(unsigned int sampleLineNumber,
                                       const std::vector<unsigned int>& childrenLineNumbers) const;

    /** Soma with neurite parent error message */
    std::string ERROR_SOMA_WITH_NEURITE_PARENT(const unsigned int lineNumber) const;

    /** Repeated section id error message */
    std::string ERROR_REPEATED_ID(unsigned int originalId,
                                  unsigned int originalLineNumber,
                                  unsigned int newLineNumber) const;

    /** Section self parent error message */
    std::string ERROR_SELF_PARENT(unsigned int lineNumber) const;

    /** Undefined soma error message */
    std::string ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA(const std::string&) const;

    /** Missing mitochondria parent section error message */
    std::string ERROR_MISSING_MITO_PARENT(int mitoParentId) const;

    ////////////////////////////////////////////////////////////////////////////////
    //             NEUROLUCIDA
    ////////////////////////////////////////////////////////////////////////////////
    /** Already defined soma error message */
    std::string ERROR_SOMA_ALREADY_DEFINED(long unsigned int lineNumber) const;

    /** Parsing ASC points error message */
    std::string ERROR_PARSING_POINT(long unsigned int lineNumber, const std::string& point) const;

    /** Unknown ASC token error message */
    std::string ERROR_UNKNOWN_TOKEN(long unsigned int lineNumber, const std::string& token) const;

    /** Unexpected ASC token error message */
    std::string ERROR_UNEXPECTED_TOKEN(long unsigned int lineNumber,
                                       const std::string& expected,
                                       const std::string& got,
                                       const std::string& msg) const;

    /** ASC EOF reached error message */
    std::string ERROR_EOF_REACHED(long unsigned int lineNumber) const;

    /** ASC EOF reached in neurite error message */
    std::string ERROR_EOF_IN_NEURITE(long unsigned int lineNumber) const;

    /** ASC unbalanced parents error message */
    std::string ERROR_EOF_UNBALANCED_PARENS(long unsigned int lineNumber) const;

    /** Incompatible flags error message */
    std::string ERROR_UNCOMPATIBLE_FLAGS(morphio::Option flag1, morphio::Option flag2) const;

    ////////////////////////////////////////////////////////////////////////////////
    //              WRITERS
    ////////////////////////////////////////////////////////////////////////////////

    /** Unsupported morphology section type error message */
    std::string ERROR_UNSUPPORTED_SECTION_TYPE(const SectionType& type) const;

    /** Wrong morphology file extension error message */
    std::string ERROR_WRONG_EXTENSION(const std::string& filename) const;

    /** Vector length mismatch error message */
    std::string ERROR_VECTOR_LENGTH_MISMATCH(const std::string& vec1,
                                             size_t length1,
                                             const std::string& vec2,
                                             size_t length2) const;

    /** Can't write perimeter data to SWC, ASC error message */
    std::string ERROR_PERIMETER_DATA_NOT_WRITABLE();
    /** Single section child SWC error message */
    std::string ERROR_ONLY_CHILD_SWC_WRITER(unsigned int parentId) const;

    /** Single point soma must have one point */
    std::string ERROR_SOMA_INVALID_SINGLE_POINT() const;

    /** Multiple points for single point soma */
    std::string ERROR_SOMA_INVALID_THREE_POINT_CYLINDER() const;

    /** Contour soma must have at least 3 points. */
    std::string ERROR_SOMA_INVALID_CONTOUR() const;


    ////////////////////////////////////////////////////////////////////////////////
    //              WARNINGS
    ////////////////////////////////////////////////////////////////////////////////

    /** Writing of mitochondria is not supported warning message */
    std::string WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED() const;
    /** Writing without soma warning message */
    std::string WARNING_WRITE_NO_SOMA() const;
    /** Writing empty morphology warning message */
    std::string WARNING_WRITE_EMPTY_MORPHOLOGY() const;
    /** Soma not found warning message */
    std::string WARNING_NO_SOMA_FOUND() const;
    /** Writing zero diameter warning message */
    std::string WARNING_ZERO_DIAMETER(long unsigned int lineNumber) const;
    /** Writing disconnected neurite warning message */
    std::string WARNING_DISCONNECTED_NEURITE(long unsigned int lineNumber) const;
    /** Writing wrong duplicate warning message */
    std::string WARNING_WRONG_DUPLICATE(const std::shared_ptr<morphio::mut::Section>& current,
                                        const std::shared_ptr<morphio::mut::Section>& parent) const;
    /** Writing empty section warning message */
    std::string WARNING_APPENDING_EMPTY_SECTION(std::shared_ptr<morphio::mut::Section>);
    /** Writing single child section warning message */
    std::string WARNING_ONLY_CHILD(unsigned int parentId, unsigned int childId) const;

    /** Soma does not conform NeuroMorpho warning message */
    std::string WARNING_NEUROMORPHO_SOMA_NON_CONFORM(const std::string&) const;

    /** Wrong root point warning message */
    std::string WARNING_WRONG_ROOT_POINT(
        const std::vector<unsigned int>& childrenLineNumbers) const;

    /** Soma is undefined*/
    std::string WARNING_UNDEFINED_SOMA() const;

    /**  Soma must be a contour for ASC and H5 */
    std::string WARNING_SOMA_NON_CONTOUR() const;

    /* Soma must be stacked cylinders or a point */
    std::string WARNING_SOMA_NON_CYLINDER_OR_POINT() const;

  private:
    std::string _uri;
};

}  // namespace readers

std::shared_ptr<ErrorAndWarningHandler> getErrorHandler();

}  // namespace morphio
