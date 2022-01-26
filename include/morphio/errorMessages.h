#pragma once

#include <map>        // std::map
#include <memory>     // std::shared_ptr
#include <set>        // std::set
#include <stdexcept>  // std::out_of_range
#include <string>     // std::string
#include <vector>     // std::vector

#include <morphio/enums.h>        // Warning, Option
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

/** Debug info for error messages **/
struct DebugInfo {
  public:
    /** Constructor

        \param filename morphology filename.
     */
    explicit DebugInfo(std::string filename = "")
        : _filename(filename) {}

    /** Stores section's line number within morphology file */
    void setLineNumber(uint32_t sectionId, unsigned int line) {
        _lineNumbers[sectionId] = static_cast<int>(line);
    }

    /** Get section's line number within morphology file */
    int32_t getLineNumber(uint32_t sectionId) const {
        try {
            return _lineNumbers.at(sectionId);
        } catch (const std::out_of_range&) {
            return -1;
        }
    }
    /** Morphology filename */
    std::string _filename;

  private:
    std::map<unsigned int, int> _lineNumbers;
};

// TODO: this shouldn't be global static
static std::set<Warning> _ignoredWarnings;

/** A sample of section for error reporting, includes its position (line) within the file. **/
struct Sample {
    Sample() = default;

    explicit Sample(const char* line, unsigned int lineNumber_)
        : lineNumber(lineNumber_) {
        floatType radius = -1.;
        int int_type = -1;
#ifdef MORPHIO_USE_DOUBLE
        const auto format = "%20u%20d%20lg%20lg%20lg%20lg%20d";
#else
        const auto format = "%20u%20d%20f%20f%20f%20f%20d";
#endif
        valid = sscanf(line,
                       format,
                       &id,
                       &int_type,
                       &point[0],
                       &point[1],
                       &point[2],
                       &radius,
                       &parentId) == 7;

        type = static_cast<SectionType>(int_type);
        diameter = radius * 2;  // The point array stores diameters.
    }

    floatType diameter = -1.;
    bool valid = false;
    Point point;
    SectionType type = SECTION_UNDEFINED;
    int parentId = -1;
    unsigned int id = 0;
    unsigned int lineNumber = 0;
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
    static bool isIgnored(Warning warning);

    /** Returns a link to a line number within the morphology file **/
    std::string errorLink(long unsigned int lineNumber, ErrorLevel errorLevel) const {
        std::map<ErrorLevel, std::string> SEVERITY{{ErrorLevel::INFO, "info"},
                                                   {ErrorLevel::WARNING, "warning"},
                                                   {ErrorLevel::ERROR, "error"}};

        const std::map<ErrorLevel, std::string> COLOR{{ErrorLevel::INFO, "\033[1;34m"},
                                                      {ErrorLevel::WARNING, "\033[1;33m"},
                                                      {ErrorLevel::ERROR, "\033[1;31m"}};

        const std::string COLOR_END("\033[0m");

        return COLOR.at(errorLevel) + _uri + ":" + std::to_string(lineNumber) + ":" +
               SEVERITY.at(errorLevel) + COLOR_END;
    }

    /** Generate an error message. */
    std::string errorMsg(long unsigned int lineNumber,
                         ErrorLevel errorLevel,
                         std::string msg = "") const;

    ////////////////////////////////////////////////////////////////////////////////
    //              ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    /** Opening file error message */
    std::string ERROR_OPENING_FILE() const;

    /** Non parsable line error message */
    std::string ERROR_LINE_NON_PARSABLE(long unsigned int lineNumber) const;

    /** Unsupported morphology section type error message */
    std::string ERROR_UNSUPPORTED_SECTION_TYPE(long unsigned int lineNumber,
                                               const SectionType& type) const;

    /** Unsupported vasculature section type error message */
    std::string ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(long unsigned int lineNumber,
                                                           const VascularSectionType& type) const;

    /** Multiple somas error message */
    std::string ERROR_MULTIPLE_SOMATA(const std::vector<Sample>& somata) const;

    /** Missing section parent error message */
    std::string ERROR_MISSING_PARENT(const Sample& sample) const;

    /** Bifurcating soma error message */
    std::string ERROR_SOMA_BIFURCATION(const Sample& sample,
                                       const std::vector<Sample>& children) const;

    /** Soma with neurite parent error message */
    std::string ERROR_SOMA_WITH_NEURITE_PARENT(const Sample& sample) const;

    /** Repeated section id error message */
    std::string ERROR_REPEATED_ID(const Sample& originalSample, const Sample& newSample) const;

    /** Section self parent error message */
    std::string ERROR_SELF_PARENT(const Sample& sample) const;

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

    /** Cant write perimeter data to SWC,ASC error message */
    std::string ERROR_PERIMETER_DATA_NOT_WRITABLE();
    /** Single section child SWC error message */
    std::string ERROR_ONLY_CHILD_SWC_WRITER(unsigned int parentId) const;


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
    std::string WARNING_ZERO_DIAMETER(const Sample& sample) const;
    /** Writing disconnected neurite warning message */
    std::string WARNING_DISCONNECTED_NEURITE(const Sample& sample) const;
    /** Writing wrong duplicate warning message */
    std::string WARNING_WRONG_DUPLICATE(const std::shared_ptr<morphio::mut::Section>& current,
                                        const std::shared_ptr<morphio::mut::Section>& parent) const;
    /** Writing empty section warning message */
    std::string WARNING_APPENDING_EMPTY_SECTION(std::shared_ptr<morphio::mut::Section>);
    /** Writing single child section warning message */
    std::string WARNING_ONLY_CHILD(const DebugInfo& info,
                                   unsigned int parentId,
                                   unsigned int childId) const;

    /** Soma does not conform NeuroMorpho warning message */
    std::string WARNING_NEUROMORPHO_SOMA_NON_CONFORM(const Sample& root,
                                                     const Sample& child1,
                                                     const Sample& child2);

    /** Wrong root point warning message */
    std::string WARNING_WRONG_ROOT_POINT(const std::vector<Sample>& children) const;

  private:
    std::string _uri;
};

}  // namespace readers

}  // namespace morphio
