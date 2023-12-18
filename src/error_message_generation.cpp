#include "error_message_generation.h"

#include <sstream>  // std::ostringstream

#include <morphio/errorMessages.h>

namespace {
/** Returns a link to a line number within the morphology file **/
std::string errorLink(
                      const std::string& uri,
                      long unsigned int lineNumber,
                      morphio::readers::ErrorLevel errorLevel) {
    using morphio::readers::ErrorLevel;

    const auto SEVERITY = [](ErrorLevel el){
        switch(el){
            case ErrorLevel::INFO: return "info";
            case ErrorLevel::WARNING: return "warning";
            case ErrorLevel::ERROR: return "error";
        }
        throw std::runtime_error("Unknown ErrorLevel");
    };

    auto COLOR = [](ErrorLevel el){
        switch(el){
        case ErrorLevel::INFO: return "\033[1;34m";
        case ErrorLevel::WARNING: return "\033[1;33m";
        case ErrorLevel::ERROR: return "\033[1;31m";
        }
        throw std::runtime_error("Unknown ErrorLevel");
    };

    const std::string COLOR_END("\033[0m");

    return COLOR(errorLevel) + uri + ":" + std::to_string(lineNumber) + ":" +
           SEVERITY(errorLevel) + COLOR_END;
}
}

namespace morphio {
namespace details {
using morphio::readers::ErrorLevel;

bool ErrorMessages::isIgnored(const Warning& warning) {
    auto static_handler = getErrorHandler();
    return static_handler->isIgnored(warning);
}

std::string ErrorMessages::errorMsg(long unsigned int lineNumber,
                                    ErrorLevel errorLevel,
                                    const std::string& msg) const {
    return "\n" + (_uri.empty() ? "" : errorLink(_uri, lineNumber, errorLevel) + "\n") + msg;
}

// LCOV_EXCL_START {  all the error messages are excluded from coverage

////////////////////////////////////////////////////////////////////////////////
//              ERRORS
////////////////////////////////////////////////////////////////////////////////

std::string ErrorMessages::ERROR_LINE_NON_PARSABLE(long unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Unable to parse this line");
}

std::string ErrorMessages::ERROR_UNSUPPORTED_SECTION_TYPE(long unsigned int lineNumber,
                                                          const SectionType& type) const {
    return errorMsg(lineNumber,
                    ErrorLevel::ERROR,
                    "Unsupported section type: " + std::to_string(type));
}

std::string ErrorMessages::ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(
    long unsigned int lineNumber, const VascularSectionType& type) const {
    return errorMsg(lineNumber,
                    ErrorLevel::ERROR,
                    "Unsupported section type: " + std::to_string(type));
}

std::string ErrorMessages::ERROR_MULTIPLE_SOMATA(
    const std::vector<unsigned int>& lineNumbers) const {
    std::string msg("Multiple somata found: ");
    for (auto lineNumber : lineNumbers) {
        msg += "\n" + errorMsg(lineNumber, ErrorLevel::ERROR);
    }
    return msg;
}

std::string ErrorMessages::ERROR_MISSING_PARENT(const unsigned int id,
                                                const int parentId,
                                                const unsigned int lineNumber) const {
    return errorMsg(lineNumber,
                    ErrorLevel::ERROR,
                    "Sample id: " + std::to_string(id) +
                        " refers to non-existant parent ID: " + std::to_string(parentId));
}

std::string ErrorMessages::ERROR_SOMA_BIFURCATION(
    const unsigned int sampleLineNumber,
    const std::vector<unsigned int>& childrenLineNumbers) const {
    std::string msg = errorMsg(sampleLineNumber, ErrorLevel::ERROR, "Found soma bifurcation\n");
    msg += "The following children have been found:";
    for (auto lineNumber : childrenLineNumbers) {
        msg += errorMsg(lineNumber, ErrorLevel::WARNING, "");
    }
    return msg;
}

std::string ErrorMessages::ERROR_SOMA_WITH_NEURITE_PARENT(const unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Found a soma point with a neurite as parent");
}

std::string ErrorMessages::ERROR_REPEATED_ID(const unsigned int originalId,
                                             const unsigned int originalLineNumber,
                                             const unsigned int newLineNumber) const {
    return errorMsg(newLineNumber,
                    ErrorLevel::WARNING,
                    "Repeated ID: " + std::to_string(originalId)) +
           "\nID already appears here: \n" + errorLink(_uri, originalLineNumber, ErrorLevel::INFO);
}

std::string ErrorMessages::ERROR_SELF_PARENT(const unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Parent ID can not be itself");
}

std::string ErrorMessages::ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA(const std::string& method) const {
    return "Cannot call: " + method + " on soma of type UNDEFINED";
}

std::string ErrorMessages::ERROR_MISSING_MITO_PARENT(int mitoParentId) const {
    return "While trying to append new mitochondria section.\n"
           "Mitochondrial parent section: " +
           std::to_string(mitoParentId) + " does not exist.";
}

////////////////////////////////////////////////////////////////////////////////
//             NEUROLUCIDA
////////////////////////////////////////////////////////////////////////////////
std::string ErrorMessages::ERROR_SOMA_ALREADY_DEFINED(long unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "A soma is already defined");
}

std::string ErrorMessages::ERROR_PARSING_POINT(long unsigned int lineNumber,
                                               const std::string& point) const {
    return errorMsg(lineNumber,
                    ErrorLevel::ERROR,
                    "Error converting: \"" + point + "\" to floatType");
}

std::string ErrorMessages::ERROR_UNKNOWN_TOKEN(long unsigned int lineNumber,
                                               const std::string& token) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Unexpected token: " + token);
}

std::string ErrorMessages::ERROR_UNEXPECTED_TOKEN(long unsigned int lineNumber,
                                                  const std::string& expected,
                                                  const std::string& got,
                                                  const std::string& msg) const {
    return errorMsg(lineNumber,
                    ErrorLevel::ERROR,
                    "Unexpected token\nExpected: " + expected + " but got " + got + " " + msg);
}

std::string ErrorMessages::ERROR_EOF_REACHED(long unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Can't iterate past the end");
}

std::string ErrorMessages::ERROR_EOF_IN_NEURITE(long unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Hit end of file while consuming a neurite");
}

std::string ErrorMessages::ERROR_EOF_UNBALANCED_PARENS(long unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Hit end of file before balanced parens");
}

std::string ErrorMessages::ERROR_UNCOMPATIBLE_FLAGS(morphio::Option flag1,
                                                    morphio::Option flag2) const {
    return errorMsg(0,
                    ErrorLevel::ERROR,
                    "Modifiers: " + std::to_string(flag1) + " and : " + std::to_string(flag2) +
                        " are incompatible");
}

////////////////////////////////////////////////////////////////////////////////
//              WRITERS
////////////////////////////////////////////////////////////////////////////////

std::string ErrorMessages::ERROR_UNSUPPORTED_SECTION_TYPE(const SectionType& type) const {
    return ("Attempted to write unsupported section type: " + std::to_string(type) +
            ".\n"
            "Please try writing to a different format that supports the section type.");
}

std::string ErrorMessages::ERROR_WRONG_EXTENSION(const std::string& filename) const {
    return "Filename: " + filename + " must have one of the following extensions: swc, asc or h5";
}

std::string ErrorMessages::ERROR_VECTOR_LENGTH_MISMATCH(const std::string& vec1,
                                                        size_t length1,
                                                        const std::string& vec2,
                                                        size_t length2) const {
    std::string msg("Vector length mismatch: \nLength " + vec1 + ": " + std::to_string(length1) +
                    "\nLength " + vec2 + ": " + std::to_string(length2));
    if (length1 == 0 || length2 == 0) {
        msg += "\nTip: Did you forget to fill vector: " + (length1 == 0 ? vec1 : vec2) + " ?";
    }

    return msg;
}

std::string ErrorMessages::ERROR_PERIMETER_DATA_NOT_WRITABLE() {
    return "Cannot write a file with perimeter data to ASC or SWC format";
}

std::string ErrorMessages::ERROR_ONLY_CHILD_SWC_WRITER(unsigned int parentId) const {
    return ("Section " + std::to_string(parentId) +
            " has a single child section. "
            "Single child section are not allowed when writing to SWC format. "
            "Please sanitize the morphology first.\n"
            "Tip: you can use 'removeUnifurcations() (C++) / remove_unifurcations() (python)'");
}

std::string ErrorMessages::ERROR_SOMA_INVALID_SINGLE_POINT() const {
    return "Single point soma must have one point";
}

std::string ErrorMessages::ERROR_SOMA_INVALID_THREE_POINT_CYLINDER() const {
    return "Multiple points for single point soma";
}

std::string ErrorMessages::ERROR_SOMA_INVALID_CONTOUR() const {
    return "Contour soma must have at least 3 points.";
}

////////////////////////////////////////////////////////////////////////////////
//              WARNINGS
////////////////////////////////////////////////////////////////////////////////
std::string ErrorMessages::WARNING_APPENDING_EMPTY_SECTION(unsigned long sectionId) const {
    return errorMsg(0,
                    ErrorLevel::WARNING,
                    "Warning: appending empty section with id: " + std::to_string(sectionId));
}

std::string ErrorMessages::WARNING_NEUROMORPHO_SOMA_NON_CONFORM(const std::string& s) const {
    return errorMsg(0, ErrorLevel::WARNING, s);
}

// LCOV_EXCL_STOP }

}  // namespace details
}  // namespace morphio
