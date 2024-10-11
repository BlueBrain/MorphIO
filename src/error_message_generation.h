/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

#include <morphio/errorMessages.h>
#include <morphio/mut/section.h>       // morphio::mut::Section
#include <morphio/warning_handling.h>  // ErrorAndWarningHandler

namespace morphio {
namespace details {

/** Class that can generate error messages and holds a collection of predefined errors
    messages **/
class ErrorMessages
{
  public:
    ErrorMessages() = default;

    /** Constructor.

       \param uri path to a morphology file.
     */
    explicit ErrorMessages(std::string uri)
        : _uri(std::move(uri)) {}

    /** Is the output of the warning ignored */
    static bool isIgnored(const enums::Warning& warning);

    ////////////////////////////////////////////////////////////////////////////////
    //              ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    /** Non parsable line error message */
    std::string ERROR_LINE_NON_PARSABLE(long unsigned int lineNumber) const;

    /** Unsupported morphology section type error message */
    std::string ERROR_UNSUPPORTED_SECTION_TYPE(long unsigned int lineNumber,
                                               const enums::SectionType& type) const;

    /** Unsupported vasculature section type error message */
    std::string ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(
        long unsigned int lineNumber, const enums::VascularSectionType& type) const;

    /** Multiple somas error message */
    std::string ERROR_MULTIPLE_SOMATA(const std::vector<unsigned int>& lineNumbers) const;

    /** Missing section parent error message */
    std::string ERROR_MISSING_PARENT(unsigned int id, int parentId, unsigned int lineNumber) const;

    /** Bifurcating soma error message */
    std::string ERROR_SOMA_BIFURCATION(unsigned int sampleLineNumber,
                                       const std::vector<unsigned int>& childrenLineNumbers) const;

    /** Soma with neurite parent error message */
    std::string ERROR_SOMA_WITH_NEURITE_PARENT(unsigned int lineNumber) const;

    /** Repeated section id error message */
    std::string ERROR_REPEATED_ID(unsigned int originalId,
                                  unsigned int originalLineNumber,
                                  unsigned int newLineNumber) const;

    /** Section self parent error message */
    std::string ERROR_SELF_PARENT(unsigned int lineNumber) const;

    /** The end of the file was reached before parsing finshed */
    std::string EARLY_END_OF_FILE(long unsigned int lineNumber) const;

    /** Undefined soma error message */
    std::string ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA(const std::string&) const;

    /** Missing mitochondria parent section error message */
    std::string ERROR_MISSING_MITO_PARENT(int mitoParentId) const;

    ////////////////////////////////////////////////////////////////////////////////
    //             SWC
    ////////////////////////////////////////////////////////////////////////////////
    /** A negative ID is used in SWC */
    std::string ERROR_NEGATIVE_ID(long unsigned int lineNumber) const;

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
    std::string ERROR_UNCOMPATIBLE_FLAGS(enums::Option flag1, enums::Option flag2) const;

    ////////////////////////////////////////////////////////////////////////////////
    //              WRITERS
    ////////////////////////////////////////////////////////////////////////////////

    /** Morphology is empty */
    std::string ERROR_EMPTY_MORPHOLOGY() const;

    /** Unsupported morphology section type error message */
    std::string ERROR_UNSUPPORTED_SECTION_TYPE(const enums::SectionType& type) const;

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

  private:
    std::string _uri;
};

}  // namespace details
}  // namespace morphio
