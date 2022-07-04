#include <iostream>  // std::cerr
#include <sstream>   // std::ostringstream
#include <string>
#include <vector>

#include <morphio/errorMessages.h>

namespace {

struct ErrorHandler{
    int MORPHIO_MAX_N_WARNINGS = 100;
    bool MORPHIO_RAISE_WARNINGS = false;
    int errorCount = 0;
    std::set<morphio::Warning> ignoredWarnings;

    void printError(const morphio::Warning& warning, const std::string& msg) {
        if (isIgnored(warning) || MORPHIO_MAX_N_WARNINGS == 0) {
            return;
        }

        if (MORPHIO_RAISE_WARNINGS) {
            throw morphio::MorphioError(msg);
        }

        if (MORPHIO_MAX_N_WARNINGS < 0 || errorCount <= MORPHIO_MAX_N_WARNINGS) {
            std::cerr << msg << '\n';
            if (errorCount == MORPHIO_MAX_N_WARNINGS) {
                std::cerr << "Maximum number of warning reached. Next warnings "
                    "won't be displayed.\n"
                    "You can change this number by calling:\n"
                    "\t- C++: set_maximum_warnings(int)\n"
                    "\t- Python: morphio.set_maximum_warnings(int)\n"
                    "0 will print no warning. -1 will print them all\n";
            }
            ++errorCount;
        }
    }

    bool isIgnored(morphio::Warning warning) {
        return ignoredWarnings.find(warning) != ignoredWarnings.end();
    }

    void set_ignored_warning(morphio::Warning warning, bool ignore) {
        if (ignore) {
            ignoredWarnings.insert(warning);
        } else {
            ignoredWarnings.erase(warning);
        }
    }
};

ErrorHandler& getErrorHandler(){
    static ErrorHandler error_handler;
    return error_handler;
}

} // namespace

namespace morphio {

/**
   Controls the maximum number of warning to be printed on screen
   0 will print no warning
   -1 will print them all
**/
void set_maximum_warnings(int n_warnings) {
    auto& errorHandler = getErrorHandler();
    errorHandler.MORPHIO_MAX_N_WARNINGS = n_warnings;
}

/**
   Whether to raise warning as errors
**/
void set_raise_warnings(bool is_raise) {
    auto& errorHandler = getErrorHandler();
    errorHandler.MORPHIO_RAISE_WARNINGS = is_raise;
}

void set_ignored_warning(Warning warning, bool ignore) {
    auto& errorHandler = getErrorHandler();
    errorHandler.set_ignored_warning(warning, ignore);
}

void set_ignored_warning(const std::vector<Warning>& warnings, bool ignore) {
    for (auto warning : warnings) {
        set_ignored_warning(warning, ignore);
    }
}

void printError(Warning warning, const std::string& msg) {
    auto& errorHandler = getErrorHandler();
    errorHandler.printError(warning, msg);
}

namespace readers {

bool ErrorMessages::isIgnored(const Warning& warning) {
    auto& errorHandler = getErrorHandler();
    return errorHandler.isIgnored(warning);
}

std::string ErrorMessages::errorMsg(long unsigned int lineNumber,
                                    ErrorLevel errorLevel,
                                    std::string msg) const {
    return "\n" + (_uri.empty() ? "" : errorLink(lineNumber, errorLevel) + "\n") + msg;
}

// LCOV_EXCL_START {  all the error messages are excluded from coverage

////////////////////////////////////////////////////////////////////////////////
//              ERRORS
////////////////////////////////////////////////////////////////////////////////

std::string ErrorMessages::ERROR_OPENING_FILE() const {
    return "Error opening morphology file:\n" + errorMsg(0, ErrorLevel::ERROR);
}

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
           "\nID already appears here: \n" + errorLink(originalLineNumber, ErrorLevel::INFO);
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
std::string ErrorMessages::WARNING_WRITE_NO_SOMA() const {
    return errorMsg(0, ErrorLevel::WARNING, "Warning: writing file without a soma");
}

std::string ErrorMessages::WARNING_WRITE_EMPTY_MORPHOLOGY() const {
    return errorMsg(0,
                    ErrorLevel::WARNING,
                    "Warning: Skipping an attempt to write an empty morphology.");
}

std::string ErrorMessages::WARNING_NO_SOMA_FOUND() const {
    return errorMsg(0, ErrorLevel::WARNING, "Warning: no soma found in file");
}

std::string ErrorMessages::WARNING_ZERO_DIAMETER(const long unsigned int lineNumber) const {
    return errorMsg(lineNumber, ErrorLevel::WARNING, "Warning: zero diameter in file");
}

std::string ErrorMessages::WARNING_DISCONNECTED_NEURITE(const long unsigned int lineNumber) const {
    return errorMsg(lineNumber,
                    ErrorLevel::WARNING,
                    "Warning: found a disconnected neurite.\n"
                    "Neurites are not supposed to have parentId: -1\n"
                    "(although this is normal if this neuron has no soma)");
}

std::string ErrorMessages::WARNING_APPENDING_EMPTY_SECTION(
    std::shared_ptr<morphio::mut::Section> section) {
    return errorMsg(0,
                    ErrorLevel::WARNING,
                    "Warning: appending empty section with id: " + std::to_string(section->id()));
}

std::string ErrorMessages::WARNING_WRONG_DUPLICATE(
    const std::shared_ptr<morphio::mut::Section>& current,
    const std::shared_ptr<morphio::mut::Section>& parent) const {
    std::string msg("Warning: while appending section: " + std::to_string(current->id()) +
                    " to parent: " + std::to_string(parent->id()));

    if (parent->points().empty()) {
        return errorMsg(0, ErrorLevel::WARNING, msg + "\nThe parent section is empty.");
    }

    if (current->points().empty()) {
        return errorMsg(0,
                        ErrorLevel::WARNING,
                        msg +
                            "\nThe current section has no points. It should at "
                            "least contains "
                            "parent section last point");
    }

    auto p0 = parent->points()[parent->points().size() - 1];
    auto p1 = current->points()[0];
    auto d0 = parent->diameters()[parent->diameters().size() - 1];
    auto d1 = current->diameters()[0];

    std::ostringstream oss;
    oss << msg
        << "\nThe section first point should be parent section last point: "
           "\n        : X Y Z Diameter"
           "\nparent last point :["
        << std::to_string(p0[0]) << ", " << std::to_string(p0[1]) << ", " << std::to_string(p0[2])
        << ", " << std::to_string(d0) << "]\nchild first point :[" << std::to_string(p1[0]) << ", "
        << std::to_string(p1[1]) << ", " << std::to_string(p1[2]) << ", " << std::to_string(d1)
        << "]\n";
    return errorMsg(0, ErrorLevel::WARNING, oss.str());
}

std::string ErrorMessages::WARNING_ONLY_CHILD(const DebugInfo& info,
                                              unsigned int parentId,
                                              unsigned int childId) const {
    int parentLine = info.getLineNumber(parentId);
    int childLine = info.getLineNumber(childId);
    std::string parentMsg;
    std::string childMsg;

    if (parentLine > -1 && childLine > -1) {
        parentMsg = " starting at:\n" +
                    errorLink(static_cast<size_t>(parentLine), ErrorLevel::INFO) + "\n";
        childMsg = " starting at:\n" +
                   errorLink(static_cast<size_t>(childLine), ErrorLevel::WARNING) + "\n";
    }

    std::ostringstream oss;
    oss << "Warning: section " << childId << childMsg << " is the only child of "
        << "section: " << std::to_string(parentId) << parentMsg
        << "\nIt will be merged with the parent section";

    return errorMsg(0, ErrorLevel::WARNING, oss.str());
}

std::string ErrorMessages::WARNING_NEUROMORPHO_SOMA_NON_CONFORM(const Point& rootPoint,
                                                                const floatType rootDiameter,
                                                                const Point& child1Point,
                                                                const floatType child1Diameter,
                                                                const Point& child2Point,
                                                                const floatType child2Diameter) {
    std::stringstream ss;

    // Return val1 and highlight it with some color if val1 != val2
    auto _col = [](floatType val1, floatType val2) {
        bool is_ok = std::fabs(val1 - val2) < morphio::epsilon;
        if (is_ok) {
            return std::to_string(val1);
        }
        return "\033[1;33m" + std::to_string(val1) + " (exp. " + std::to_string(val2) + ")\033[0m";
    };

    floatType x = rootPoint[0];
    floatType y = rootPoint[1];
    floatType z = rootPoint[2];
    floatType r = rootDiameter / 2;

    ss << "Warning: the soma does not conform the three point soma spec\n"
          "The only valid neuro-morpho soma is:\n"
          "1 1 x   y   z r -1\n"
          "2 1 x (y-r) z r  1\n"
          "3 1 x (y+r) z r  1\n\n"

          "Got:\n"
          "1 1 "
       << x << ' ' << y << ' ' << z << ' ' << r
       << " -1\n"
          "2 1 "
       << _col(child1Point[0], x) << ' ' << _col(child1Point[1], y - r) << ' '
       << _col(child1Point[2], z) << ' ' << _col(child1Diameter / 2, r)
       << " 1\n"
          "3 1 "
       << _col(child2Point[0], x) << ' ' << _col(child2Point[1], y + r) << ' '
       << _col(child2Point[2], z) << ' ' << _col(child2Diameter / 2, r) << " 1\n";
    return errorMsg(0, ErrorLevel::WARNING, ss.str());
}

std::string ErrorMessages::WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED() const {
    return errorMsg(0,
                    ErrorLevel::WARNING,
                    "Warning: this cell has mitochondria, they cannot be saved in "
                    " ASC or SWC format. Please use H5 if you want to save them.");
}

std::string ErrorMessages::WARNING_WRONG_ROOT_POINT(
    const std::vector<unsigned int>& childrenLineNumbers) const {
    std::ostringstream oss;
    oss << "Warning: with a 3 points soma, neurites must be connected to the first soma "
           "point:";
    for (const auto& lineNumber : childrenLineNumbers) {
        oss << errorMsg(lineNumber, ErrorLevel::WARNING, "");
    }
    return oss.str();
}

std::string ErrorMessages::WARNING_SOMA_NON_CONTOUR() const {
    return errorMsg(0,
                    ErrorLevel::WARNING,
                    "Soma must be a contour for ASC and H5: see "
                    "https://github.com/BlueBrain/MorphIO/issues/457");
}

std::string ErrorMessages::WARNING_SOMA_NON_CYLINDER_OR_POINT() const {
    return errorMsg(0,
                    ErrorLevel::WARNING,
                    "Soma must be stacked cylinders or a point: see "
                    "https://github.com/BlueBrain/MorphIO/issues/457");
}

// LCOV_EXCL_STOP }

}  // namespace readers

}  // namespace morphio
