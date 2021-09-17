#pragma once

#include <map>     // std::map
#include <memory>  // std::shared_ptr
#include <set>     // std::set
#include <string>  // std::string

#include <morphio/mut/modifiers.h>
#include <morphio/mut/section.h>

namespace morphio {
/**
   Set the maximum number of warnings to be printed on screen
**/
void set_maximum_warnings(int n_warnings);
void set_raise_warnings(bool is_raise);
void set_ignored_warning(Warning warning, bool ignore = true);
void set_ignored_warning(const std::vector<Warning>& warning, bool ignore = true);

void printError(Warning warning, const std::string& msg);

namespace readers {
enum ErrorLevel { INFO, WARNING, ERROR };

struct DebugInfo {
  public:
    DebugInfo(std::string filename = "")
        : _filename(filename) {}

    void setLineNumber(uint32_t sectionId, unsigned int line) {
        _lineNumbers[sectionId] = static_cast<int>(line);
    }

    int32_t getLineNumber(uint32_t sectionId) const {
        try {
            return _lineNumbers.at(sectionId);
        } catch (const std::out_of_range&) {
            return -1;
        }
    }
    std::string _filename;

  private:
    std::map<unsigned int, int> _lineNumbers;
};

static std::set<Warning> _ignoredWarnings;

struct Sample {
    Sample()
        : valid(false)
        , type(SECTION_UNDEFINED)
        , parentId(-1)
        , lineNumber(0) {}

    explicit Sample(const char* line, unsigned int lineNumber_)
        : lineNumber(lineNumber_) {
        floatType radius;
        int int_type;
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

    floatType diameter;
    bool valid;
    Point point;  // x, y, z and diameter
    SectionType type;
    int parentId;
    unsigned int id;
    unsigned int lineNumber;
};

class ErrorMessages
{
  public:
    ErrorMessages() {}
    ErrorMessages(const std::string& uri)
        : _uri(uri) {}

    /**
       Is the output of the warning ignored
    **/
    static bool isIgnored(Warning warning);

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

    std::string errorMsg(long unsigned int lineNumber,
                         ErrorLevel errorLevel,
                         std::string msg = "") const;

    ////////////////////////////////////////////////////////////////////////////////
    //              ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    std::string ERROR_OPENING_FILE() const;

    std::string ERROR_LINE_NON_PARSABLE(long unsigned int lineNumber) const;

    std::string ERROR_UNSUPPORTED_SECTION_TYPE(long unsigned int lineNumber,
                                               const SectionType& type) const;

    std::string ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE(long unsigned int lineNumber,
                                                           const VascularSectionType& type) const;

    std::string ERROR_MULTIPLE_SOMATA(const std::vector<Sample>& somata) const;

    std::string ERROR_MISSING_PARENT(const Sample& sample) const;

    std::string ERROR_SOMA_BIFURCATION(const Sample& sample,
                                       const std::vector<Sample>& children) const;

    std::string ERROR_SOMA_WITH_NEURITE_PARENT(const Sample& sample) const;

    std::string ERROR_REPEATED_ID(const Sample& originalSample, const Sample& newSample) const;

    std::string ERROR_SELF_PARENT(const Sample& sample) const;

    std::string ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA(const std::string&) const;

    std::string ERROR_MISSING_MITO_PARENT(int mitoParentId) const;

    ////////////////////////////////////////////////////////////////////////////////
    //             NEUROLUCIDA
    ////////////////////////////////////////////////////////////////////////////////
    std::string ERROR_SOMA_ALREADY_DEFINED(long unsigned int lineNumber) const;

    std::string ERROR_PARSING_POINT(long unsigned int lineNumber, const std::string& point) const;

    std::string ERROR_UNKNOWN_TOKEN(long unsigned int lineNumber, const std::string& token) const;

    std::string ERROR_UNEXPECTED_TOKEN(long unsigned int lineNumber,
                                       const std::string& expected,
                                       const std::string& got,
                                       const std::string& msg) const;

    std::string ERROR_EOF_REACHED(long unsigned int lineNumber) const;

    std::string ERROR_EOF_IN_NEURITE(long unsigned int lineNumber) const;

    std::string ERROR_EOF_UNBALANCED_PARENS(long unsigned int lineNumber) const;

    std::string ERROR_UNCOMPATIBLE_FLAGS(morphio::Option flag1, morphio::Option flag2) const;

    ////////////////////////////////////////////////////////////////////////////////
    //              WRITERS
    ////////////////////////////////////////////////////////////////////////////////

    std::string ERROR_WRONG_EXTENSION(const std::string& filename) const;

    std::string ERROR_VECTOR_LENGTH_MISMATCH(const std::string& vec1,
                                             size_t length1,
                                             const std::string& vec2,
                                             size_t length2) const;

    std::string ERROR_PERIMETER_DATA_NOT_WRITABLE();
    std::string ERROR_ONLY_CHILD_SWC_WRITER(unsigned int parentId) const;


    ////////////////////////////////////////////////////////////////////////////////
    //              WARNINGS
    ////////////////////////////////////////////////////////////////////////////////

    std::string WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED() const;
    std::string WARNING_WRITE_NO_SOMA() const;
    std::string WARNING_WRITE_EMPTY_MORPHOLOGY() const;
    std::string WARNING_NO_SOMA_FOUND() const;
    std::string WARNING_ZERO_DIAMETER(const Sample& sample) const;
    std::string WARNING_DISCONNECTED_NEURITE(const Sample& sample) const;
    std::string WARNING_WRONG_DUPLICATE(const std::shared_ptr<morphio::mut::Section>& current,
                                        const std::shared_ptr<morphio::mut::Section>& parent) const;
    std::string WARNING_APPENDING_EMPTY_SECTION(std::shared_ptr<morphio::mut::Section>);
    std::string WARNING_ONLY_CHILD(const DebugInfo& info,
                                   unsigned int parentId,
                                   unsigned int childId) const;

    std::string WARNING_NEUROMORPHO_SOMA_NON_CONFORM(const Sample& root,
                                                     const Sample& child1,
                                                     const Sample& child2);

    std::string WARNING_WRONG_ROOT_POINT(const std::vector<Sample>& children) const;

  private:
    std::string _uri;
};

}  // namespace readers

}  // namespace morphio
