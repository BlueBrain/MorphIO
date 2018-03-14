#pragma once

#include <string>

#include "morphologySWC.h"


namespace morphio
{
namespace plugin
{

enum ErrorLevel {
    INFO,
    WARNING,
    ERROR
};

class ErrorMessages {
public:
    ErrorMessages(){};
    ErrorMessages(const std::string& uri) : _uri(uri) {};

    const std::string errorLink(int lineNumber, ErrorLevel errorLevel) const {
        std::map<ErrorLevel, std::string> SEVERITY{
            {ErrorLevel::INFO, "info"},
            {ErrorLevel::WARNING, "warning"},
            {ErrorLevel::ERROR, "error"}};

        const std::map<ErrorLevel, std::string> COLOR{
            {ErrorLevel::INFO, "\e[1;34m"},
            {ErrorLevel::WARNING, "\e[1;33m"},
            {ErrorLevel::ERROR, "\e[1;31m"}};

        const std::string COLOR_END("\e[0m");


        return  COLOR.at(errorLevel) + _uri + ":" + std::to_string(lineNumber) + ":" + SEVERITY.at(errorLevel) + COLOR_END;
    }

    const std::string errorMsg(int lineNumber, ErrorLevel errorLevel,
                               std::string msg = "") const {
        return "\n" +
            (_uri.empty() ? "" : errorLink(lineNumber, errorLevel) + "\n")
            + msg;
    }


    ////////////////////////////////////////////////////////////////////////////////
    //              ERRORS
    ////////////////////////////////////////////////////////////////////////////////

    const std::string ERROR_OPENING_FILE() const {
        return "Error opening morphology file:\n" + errorMsg(0, ErrorLevel::ERROR);
    };

    const std::string ERROR_LINE_NON_PARSABLE(int lineNumber) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR, "Unable to parse this line");
    }

    const std::string ERROR_MULTIPLE_SOMATA(const std::vector<swc::Sample>& somata) const {
        std::string msg("Multiple somata found: ");
        for(auto soma: somata)
            msg += "\n" + errorMsg(soma.lineNumber, ErrorLevel::ERROR);
        return msg;
    }

    const std::string ERROR_MISSING_PARENT(const swc::Sample& sample) const {
        return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
                        "Sample id: " + std::to_string(sample.id) +
                        " refers to non-existant parent ID: " +
                        std::to_string(sample.parentId));
    }

    std::string ERROR_SOMA_BIFURCATION(const swc::Sample& sample) const {
        return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
                        "Found soma bifurcation");
    }

    std::string ERROR_SOMA_WITH_NEURITE_PARENT(const swc::Sample& sample) const {
        return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
                        "Found a soma point with a neurite as parent");
    };

    const std::string ERROR_REPEATED_ID(const swc::Sample& originalSample, const swc::Sample& newSample) const {
        return errorMsg(newSample.lineNumber, ErrorLevel::WARNING,
                        "Repeated ID: " + std::to_string(originalSample.id)) +
            "\nID already appears here: \n" + errorLink(originalSample.lineNumber,
                                                        ErrorLevel::INFO);
    }

    const std::string ERROR_SELF_PARENT(const swc::Sample& sample) const {
        return errorMsg(sample.lineNumber, ErrorLevel::ERROR,
                        "Parent ID can not be itself");
    }


    ////////////////////////////////////////////////////////////////////////////////
    //             NEUROLUCIDA
    ////////////////////////////////////////////////////////////////////////////////
    const std::string ERROR_SOMA_ALREADY_DEFINED(int lineNumber) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "A soma is already defined");
    }

    const std::string ERROR_BROKEN_DUPLICATE(int lineNumber) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Parent point is duplicated but have a different radius");
    }


    const std::string ERROR_PARSING_POINT(int lineNumber,
                                          const std::string& point) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Error converting: \"" + point + "\" to float");
    }

    const std::string ERROR_UNKNOWN_TOKEN(int lineNumber,
                                          const std::string& token) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Unexpected token: " + token);
    }

    const std::string ERROR_UNEXPECTED_TOKEN(int lineNumber,
                                             const std::string& expected,
                                             const std::string& got,
                                             const std::string& msg) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Unexpected token\nExpected: " + expected
                        + " but got " + got + " " + msg);
    }

    const std::string ERROR_EOF_REACHED(int lineNumber) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Can't iterate past the end");
    }

    const std::string ERROR_EOF_IN_NEURITE(int lineNumber) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Hit end of file while consuming a neurite");
    }

    const std::string ERROR_EOF_UNBALANCED_PARENS(int lineNumber) const {
        return errorMsg(lineNumber, ErrorLevel::ERROR,
                        "Hit end of file before balanced parens");
    }


    ////////////////////////////////////////////////////////////////////////////////
    //              WARNINGS
    ////////////////////////////////////////////////////////////////////////////////
    std::string WARNING_NO_SOMA_FOUND() const {
        return errorMsg(0, ErrorLevel::WARNING,
                        "No soma found in file");
    }

    std::string WARNING_DISCONNECTED_NEURITE(const swc::Sample& sample) const {
        return errorMsg(sample.lineNumber, ErrorLevel::WARNING,
                        "Found a disconnected neurite.\n"
                        "Neurites are not supposed to have parentId: -1\n"
                        "(although this is normal if this neuron has no soma)");
    }

    std::string WARNING_WRONG_DUPLICATE(uint32_t id, uint32_t parentId) const {
        return errorMsg(0, ErrorLevel::WARNING,
                        "While appending section: "+std::to_string(id) +
                        " to parent: "+std::to_string(parentId) +
                        "\nThe section first point should be parent section last point");
    }

private:
    std::string _uri;
};


} // namespace plugin

} // namespace morphio
