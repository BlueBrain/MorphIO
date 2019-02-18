#include <cmath>
#include <morphio/errorMessages.h>
#include <sstream>

namespace morphio {
static int MORPHIO_MAX_N_WARNINGS = 100;

/**
   Controls the maximum number of warning to be printed on screen
   0 will print no warning
   -1 will print them all
**/
void set_maximum_warnings(int n_warnings)
{
    MORPHIO_MAX_N_WARNINGS = n_warnings;
}

void set_ignored_warning(Warning warning, bool ignore)
{
    if (ignore)
        plugin::_ignoredWarnings.insert(warning);
    else
        plugin::_ignoredWarnings.erase(warning);
}

void set_ignored_warning(const std::vector<Warning>& warnings, bool ignore)
{
    for (auto warning : warnings)
        set_ignored_warning(warning, ignore);
}

void LBERROR(Warning warning, const std::string& msg)
{
    static int error = 0;
    if (plugin::ErrorMessages::isIgnored(warning) || MORPHIO_MAX_N_WARNINGS == 0)
        return;

    if (MORPHIO_MAX_N_WARNINGS < 0 || error <= MORPHIO_MAX_N_WARNINGS) {
        std::cerr << msg << std::endl;
        if (error == MORPHIO_MAX_N_WARNINGS) {
            std::cerr << "Maximum number of warning reached. Next warnings "
                         "won't be displayed."
                      << std::endl;
            std::cerr << "You can change this number by calling:" << std::endl;
            std::cerr << "\t- C++: set_maximum_warnings(int)" << std::endl;
            std::cerr << "\t- Python: morphio.set_maximum_warnings(int)"
                      << std::endl;
            std::cerr << "0 will print no warning. -1 will print them all"
                      << std::endl;
        }
        ++error;
    }
}

namespace plugin {
bool ErrorMessages::isIgnored(Warning warning)
{
    return _ignoredWarnings.find(warning) != _ignoredWarnings.end();
}

const std::string ErrorMessages::errorMsg(int lineNumber, ErrorLevel errorLevel,
    std::string msg) const
{
    return "\n" + (_uri.empty() ? "" : errorLink(lineNumber, errorLevel) + "\n") + msg;
}

////////////////////////////////////////////////////////////////////////////////
//              ERRORS
////////////////////////////////////////////////////////////////////////////////

const std::string ErrorMessages::ERROR_OPENING_FILE() const
{
    return "Error opening morphology file:\n" + errorMsg(0, ErrorLevel::ERROR);
};

const std::string ErrorMessages::ERROR_LINE_NON_PARSABLE(int lineNumber) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR, "Unable to parse this line");
}

const std::string ErrorMessages::ERROR_MULTIPLE_SOMATA(
    const std::vector<Sample>& somata) const
{
    std::string msg("Multiple somata found: ");
    for (auto soma : somata)
        msg += "\n" + errorMsg(soma.lineNumber, ErrorLevel::ERROR);
    return msg;
}

const std::string ErrorMessages::ERROR_MISSING_PARENT(
    const Sample& sample) const
{
    return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
        "Sample id: " + std::to_string(sample.id) + " refers to non-existant parent ID: " + std::to_string(sample.parentId));
}

std::string ErrorMessages::ERROR_SOMA_BIFURCATION(
    const Sample& sample, const std::vector<Sample>& children) const
{
    std::string msg = errorMsg(sample.lineNumber, ErrorLevel::ERROR,
        "Found soma bifurcation\n");
    msg += "The following children have been found:";
    for (auto child : children)
        msg += errorMsg(child.lineNumber, ErrorLevel::WARNING, "");
    return msg;
}

std::string ErrorMessages::ERROR_SOMA_WITH_NEURITE_PARENT(
    const Sample& sample) const
{
    return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
        "Found a soma point with a neurite as parent");
};

const std::string ErrorMessages::ERROR_REPEATED_ID(
    const Sample& originalSample, const Sample& newSample) const
{
    return errorMsg(newSample.lineNumber, ErrorLevel::WARNING,
               "Repeated ID: " + std::to_string(originalSample.id)) +
           "\nID already appears here: \n" + errorLink(originalSample.lineNumber, ErrorLevel::INFO);
}

const std::string ErrorMessages::ERROR_SELF_PARENT(const Sample& sample) const
{
    return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
        "Parent ID can not be itself");
}

const std::string ErrorMessages::ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA(
    const std::string& method) const
{
    return "Cannot call: " + method + " on soma of type UNDEFINED";
}

const std::string ErrorMessages::ERROR_MISSING_MITO_PARENT(
    int mitoParentId) const
{
    return "While trying to append new mitochondria section.\n"
           "Mitochondrial parent section: " +
           std::to_string(mitoParentId) + " does not exist.";
}

/**
   Return val1 and highlight it with some color if val1 != val2
**/
const std::string _col(float val1, float val2)
{
    bool is_ok = std::fabs(val1 - val2) < 1e-6;
    if (is_ok)
        return std::to_string(val1);
    return "\e[1;33m" + std::to_string(val1) + " (exp. " + std::to_string(val2) + ")\e[0m";
}

////////////////////////////////////////////////////////////////////////////////
//             NEUROLUCIDA
////////////////////////////////////////////////////////////////////////////////
const std::string ErrorMessages::ERROR_SOMA_ALREADY_DEFINED(
    int lineNumber) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR, "A soma is already defined");
}

const std::string ErrorMessages::ERROR_PARSING_POINT(
    int lineNumber, const std::string& point) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR,
        "Error converting: \"" + point + "\" to float");
}

const std::string ErrorMessages::ERROR_UNKNOWN_TOKEN(
    int lineNumber, const std::string& token) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR,
        "Unexpected token: " + token);
}

const std::string ErrorMessages::ERROR_UNEXPECTED_TOKEN(
    int lineNumber, const std::string& expected, const std::string& got,
    const std::string& msg) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR,
        "Unexpected token\nExpected: " + expected + " but got " + got + " " + msg);
}

const std::string ErrorMessages::ERROR_EOF_REACHED(int lineNumber) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR,
        "Can't iterate past the end");
}

const std::string ErrorMessages::ERROR_EOF_IN_NEURITE(int lineNumber) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR,
        "Hit end of file while consuming a neurite");
}

const std::string ErrorMessages::ERROR_EOF_UNBALANCED_PARENS(
    int lineNumber) const
{
    return errorMsg(lineNumber, ErrorLevel::ERROR,
        "Hit end of file before balanced parens");
}

const std::string ErrorMessages::ERROR_UNCOMPATIBLE_FLAGS(
    morphio::Option flag1, morphio::Option flag2) const
{
    return errorMsg(0, ErrorLevel::ERROR,
        "Modifiers: " + std::to_string(flag1) + " and : " + std::to_string(flag2) + " are incompatible");
}

////////////////////////////////////////////////////////////////////////////////
//              WRITERS
////////////////////////////////////////////////////////////////////////////////

const std::string ErrorMessages::ERROR_WRONG_EXTENSION(
    const std::string filename) const
{
    return "Filename: " + filename + " must have one of the following extensions: swc, asc or h5";
};

std::string ErrorMessages::ERROR_VECTOR_LENGTH_MISMATCH(const std::string& vec1,
    int length1,
    const std::string& vec2,
    int length2) const
{
    std::string msg("Vector length mismatch: \nLength " + vec1 + ": " + std::to_string(length1) + "\nLength " + vec2 + ": " + std::to_string(length2));
    if (length1 == 0 || length2 == 0)
        msg += "\nTip: Did you forget to fill vector: " + (length1 == 0 ? vec1 : vec2) + " ?";

    return msg;
}

////////////////////////////////////////////////////////////////////////////////
//              WARNINGS
////////////////////////////////////////////////////////////////////////////////
std::string ErrorMessages::WARNING_WRITE_NO_SOMA() const
{
    return "Warning: writing file without a soma";
}

std::string ErrorMessages::WARNING_NO_SOMA_FOUND() const
{
    return errorMsg(0, ErrorLevel::WARNING, "No soma found in file");
}

std::string ErrorMessages::WARNING_DISCONNECTED_NEURITE(
    const Sample& sample) const
{
    return errorMsg(sample.lineNumber, ErrorLevel::WARNING,
        "Found a disconnected neurite.\n"
        "Neurites are not supposed to have parentId: -1\n"
        "(although this is normal if this neuron has no soma)");
}

std::string ErrorMessages::WARNING_APPENDING_EMPTY_SECTION(std::shared_ptr<morphio::mut::Section> section)
{
    return errorMsg(0, ErrorLevel::WARNING, "Appending empty section with id: " + std::to_string(section->id()));
}

std::string ErrorMessages::WARNING_WRONG_DUPLICATE(
    std::shared_ptr<morphio::mut::Section> current,
    std::shared_ptr<morphio::mut::Section> parent) const
{
    std::string msg(
        "While appending section: " + std::to_string(current->id()) + " to parent: " + std::to_string(parent->id()));

    if (parent->points().empty())
        return errorMsg(0, ErrorLevel::WARNING,
            msg + "\nThe parent section is empty.");

    if (current->points().empty())
        return errorMsg(0, ErrorLevel::WARNING,
            msg +
                "\nThe current section has no points. It should at "
                "least contains " +
                "parent section last point");

    auto p0 = parent->points()[parent->points().size() - 1];
    auto p1 = current->points()[0];
    auto d0 = parent->diameters()[parent->diameters().size() - 1];
    auto d1 = current->diameters()[0];

    return errorMsg(0, ErrorLevel::WARNING,
        msg + "\nThe section first point " + "should be parent section last point: " + "\n        : X Y Z Diameter" + "\nparent last point :[" + std::to_string(p0[0]) + ", " + std::to_string(p0[1]) + ", " + std::to_string(p0[2]) + ", " + std::to_string(d0) + "]" + "\nchild first point :[" + std::to_string(p1[0]) + ", " + std::to_string(p1[1]) + ", " + std::to_string(p1[2]) + ", " + std::to_string(d1) + "]\n");
}

const std::string ErrorMessages::WARNING_ONLY_CHILD(const DebugInfo& info,
    int parentId,
    int childId) const
{
    int parentLine = info.getLineNumber(parentId);
    int childLine = info.getLineNumber(childId);
    std::string parentMsg, childMsg;
    if (parentLine > -1 && childLine > -1) {
        parentMsg = " starting at:\n" + errorLink(parentLine, ErrorLevel::INFO) + "\n";
        childMsg = " starting at:\n" + errorLink(childLine, ErrorLevel::WARNING) + "\n";
    }

    return "\nSection: " + std::to_string(childId) + childMsg + " is the only child of " + "section: " + std::to_string(parentId) + parentMsg + "\nIt will be merged with the parent section";
}

const std::string ErrorMessages::WARNING_NEUROMORPHO_SOMA_NON_CONFORM(
    const Sample& root, const Sample& child1, const Sample& child2)
{
    float x = root.point[0], y = root.point[1], z = root.point[2],
          r = root.diameter / 2.;
    std::stringstream ss;
    ss << "The soma does not conform the three point soma spec" << std::endl;
    ss << "The only valid neuro-morpho soma is:" << std::endl;
    ss << "1 1 x   y   z r -1" << std::endl;
    ss << "2 1 x (y-r) z r  1" << std::endl;
    ss << "3 1 x (y+r) z r  1\n"
       << std::endl;

    ss << "Got:" << std::endl;
    ss << "1 1 " << x << " " << y << " " << z << " " << r << " -1" << std::endl;
    ss << "2 1 " << _col(child1.point[0], x) << " "
       << _col(child1.point[1], y - r) << " " << _col(child1.point[2], z) << " "
       << _col(child1.diameter / 2., r) << " 1" << std::endl;
    ss << "3 1 " << _col(child2.point[0], x) << " "
       << _col(child2.point[1], y + r) << " " << _col(child2.point[2], z) << " "
       << _col(child2.diameter / 2., r) << " 1" << std::endl;
    return ss.str();
}

std::string ErrorMessages::WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED() const
{
    return errorMsg(
        0, ErrorLevel::WARNING,
        "This cell has mitochondria, they cannot be saved in "
        " ASC or SWC format. Please use H5 if you want to save them.");
}

std::string ErrorMessages::WARNING_WRONG_ROOT_POINT(
    const std::vector<Sample>& children) const
{
    std::string msg =
        "With a 3 points soma, neurites must be connected to the first soma "
        "point:";
    for (auto child : children)
        msg += errorMsg(child.lineNumber, ErrorLevel::WARNING, "");
    return msg;
}

} // namespace plugin

} // namespace morphio
