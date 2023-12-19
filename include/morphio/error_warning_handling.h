/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <iostream>
#include <memory>
#include <set>
#include <sstream>  // std::ostringstream
#include <string>
#include <utility>
#include <vector>

#include <morphio/enums.h>
#include <morphio/types.h>


namespace morphio {
namespace {
/** Returns a link to a line number within the morphology file **/
std::string errorLink1(
                      const std::string& uri,
                      long unsigned int lineNumber,
                      morphio::readers::ErrorLevel errorLevel) {
    using morphio::readers::ErrorLevel;
    if(uri.empty()){
       return {};
   }

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
           SEVERITY(errorLevel) + COLOR_END + "\n";
}

std::string errorMsg(const std::string& uri,
                     long unsigned int lineNumber,
                     morphio::readers::ErrorLevel errorLevel,
                     const std::string& msg){
    return "\n" + (uri.empty() ? "" : errorLink1(uri, lineNumber, errorLevel) + "\n") + msg;
}
}


//XXX
using morphio::enums::Warning;

struct WarningMessage {
    explicit WarningMessage(std::string uri_): uri(std::move(uri_)) {}
    virtual std::string msg() const = 0;
    virtual ~WarningMessage() = default;
    virtual Warning warning() const = 0;
    std::string uri;
};

struct WarningZeroDiameter : public WarningMessage {
    WarningZeroDiameter(const std::string& uri_, uint64_t lineNumber_):
        WarningMessage(uri_), lineNumber(lineNumber_) {}
    morphio::enums::Warning warning() const final { return Warning::ZERO_DIAMETER;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: zero diameter in file";
        return "\n" + errorLink1(uri, lineNumber, errorLevel) + description;
    }

    uint64_t lineNumber;
};

struct WarningDisconnectedNeurite : public WarningMessage {
    WarningDisconnectedNeurite(const std::string& uri_, uint64_t lineNumber_):
        WarningMessage(uri_), lineNumber(lineNumber_) {}
    Warning warning() const final {return Warning::ZERO_DIAMETER;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = 
                    "Warning: found a disconnected neurite.\n"
                    "Neurites are not supposed to have parentId: -1\n"
                    "(although this is normal if this neuron has no soma)";
        return "\n" + errorLink1(uri, lineNumber, errorLevel) + description;
    }

    uint64_t lineNumber;
};

struct NoSomaFound : public WarningMessage {
    explicit NoSomaFound(const std::string& uri_): WarningMessage(uri_) {}
    Warning warning() const final {return Warning::NO_SOMA_FOUND;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: no soma found in file";
        return "\n" + errorLink1(uri, 0, errorLevel) + description;
    }
};

struct SomaNonConform : public WarningMessage {
    explicit SomaNonConform(const std::string& uri_, const std::string description_): WarningMessage(uri_), description(description_) {}
    Warning warning() const final {return Warning::SOMA_NON_CONFORM;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return "\n" + errorLink1(uri, 0, errorLevel) + description;
    }
    std::string description;
};

struct WrongRootPoint : public WarningMessage {
    explicit WrongRootPoint(const std::string& uri_, const std::vector<unsigned int> lineNumbers_): WarningMessage(uri_), lineNumbers(lineNumbers_) {}
    Warning warning() const final {return Warning::WRONG_ROOT_POINT;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        std::ostringstream oss;
        oss << "Warning: with a 3 points soma, neurites must be connected to the first soma point:";
        for (const auto& lineNumber : lineNumbers) {
            oss << "\n" + errorLink1(uri, lineNumber, errorLevel);
        }
        return oss.str();
    }
    const std::vector<unsigned int> lineNumbers;
};

struct AppendingEmptySection : public WarningMessage {
    explicit AppendingEmptySection(std::string uri_, uint32_t sectionId_):
        WarningMessage(uri_), sectionId(sectionId_) {}
    Warning warning() const final {return Warning::APPENDING_EMPTY_SECTION;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: appending empty section with id: ";
        // XXX
        (void)errorMsg(uri, 0, errorLevel, "asdf");
        return "\n" + errorLink1(uri, 0, errorLevel) + description + std::to_string(sectionId);
    }
    uint32_t sectionId;
};

struct WrongDuplicate : public WarningMessage {
    explicit WrongDuplicate(std::string uri_,
                            std::shared_ptr<morphio::mut::Section> current_,
                            std::shared_ptr<morphio::mut::Section> parent_)
        : WarningMessage(uri_)
        , current(current_)
          , parent(parent_) {}
    std::string msg() const final;

    Warning warning() const final {return Warning::WRONG_DUPLICATE;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::shared_ptr<morphio::mut::Section> current;
    std::shared_ptr<morphio::mut::Section> parent;
};

struct OnlyChild : public WarningMessage {
    explicit OnlyChild(std::string uri_,
                       unsigned int parentId_,
                       unsigned int childId_
                      ):
        WarningMessage(uri_), parentId(parentId_), childId(childId_) {}
    Warning warning() const final {return Warning::APPENDING_EMPTY_SECTION;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        std::ostringstream oss;
        oss << "Warning: section " << childId << " is the only child of "
            << "section: " << std::to_string(parentId)
            << "\nIt will be merged with the parent section";

        return errorMsg(uri, 0, errorLevel, oss.str());
    }
    unsigned int parentId;
    unsigned int childId;
};

struct WriteNoSoma : public WarningMessage {
    WriteNoSoma(): WarningMessage(std::string()) {}
    Warning warning() const final {return Warning::WRITE_NO_SOMA;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return errorMsg(uri, 0, errorLevel, "Warning: writing file without a soma");
    }
};

struct WriteEmptyMorphology : public WarningMessage {
    WriteEmptyMorphology(): WarningMessage(std::string()) {}
    Warning warning() const final {return Warning::WRITE_EMPTY_MORPHOLOGY;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return errorMsg(uri, 0, errorLevel, "Warning: Skipping an attempt to write an empty morphology.");
    }
};

struct WriteUndefinedSoma : public WarningMessage {
    WriteUndefinedSoma(): WarningMessage(std::string()) {}
    Warning warning() const final {return Warning::WRITE_UNDEFINED_SOMA;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return errorMsg(uri, 0, errorLevel, "Warning: writing soma set to SOMA_UNDEFINED");
    }
};

struct MitochondriaWriteNotSupported : public WarningMessage {
    MitochondriaWriteNotSupported(): WarningMessage(std::string()) {}
    Warning warning() const final {return Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return errorMsg(uri, 0, errorLevel,
                        "Warning: this cell has mitochondria, they cannot be saved in "
                        " ASC or SWC format. Please use H5 if you want to save them.");
    }
};

struct SomaNonContour : public WarningMessage {
    SomaNonContour(): WarningMessage(std::string()) {}
    Warning warning() const final {return Warning::SOMA_NON_CONTOUR;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return errorMsg(uri, 0, errorLevel,
                    "Soma must be a contour for ASC and H5: see "
                    "https://github.com/BlueBrain/MorphIO/issues/457");
    }
};

struct SomaNonCynlinderOrPoint : public WarningMessage {
    SomaNonCynlinderOrPoint(): WarningMessage(std::string()) {}
    Warning warning() const final {return Warning::SOMA_NON_CYLINDER_OR_POINT;}
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return errorMsg(uri, 0, errorLevel,
                    "Soma must be stacked cylinders or a point: see "
                    "https://github.com/BlueBrain/MorphIO/issues/457");
    }
};


class ErrorAndWarningHandler
{
  public:
    ErrorAndWarningHandler() = default;
    ErrorAndWarningHandler(ErrorAndWarningHandler&&) = default;
    ErrorAndWarningHandler& operator=(const ErrorAndWarningHandler&) = default;
    ErrorAndWarningHandler& operator=(ErrorAndWarningHandler&&) = default;
    ErrorAndWarningHandler(ErrorAndWarningHandler&) = default;
    virtual ~ErrorAndWarningHandler() = default;

    virtual void emit(const enums::Warning& warning, const std::string& msg) = 0;
    void emit(const std::unique_ptr<WarningMessage> wm);
    bool isIgnored(enums::Warning warning);
    void setIgnoredWarning(enums::Warning warning, bool ignore);
    int getMaxWarningCount() const; 
    void setMaxWarningCount(int warningCount);
    bool getRaiseWarnings() const; 
    void setRaiseWarnings(bool raise);

    //XXX
  //private:
    int maxWarningCount_ = 100;
    bool raiseWarnings_ = false;
    std::set<enums::Warning> ignoredWarnings_;
};


struct Caution {
    Caution() = default;
    Caution(enums::Warning level_, std::string msg_)
        : level(level_)
        , msg(msg_) {}
    enums::Warning level = enums::Warning::UNDEFINED;
    std::string msg;
};

class ErrorAndWarningHandlerCollector : public ErrorAndWarningHandler
{
public:
    void emit(const enums::Warning& warning, const std::string& msg) final {
        m.emplace_back(warning, msg);
    }

    void printAll(){
        std::cout << "printing all errors:\n";
        for(const auto& e : m){
            std::cout << "************error: " << e.msg << '\n';
        }
    }

    std::vector<Caution> getAll() const {
        return m;
    }

  private:
    std::vector<Caution> m;
};

}  // namespace morphio
