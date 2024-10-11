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
namespace details {
std::string errorLink(const std::string& uri,
                      long unsigned int lineNumber,
                      morphio::readers::ErrorLevel errorLevel);
}  // namespace details


struct WarningMessage {
    explicit WarningMessage(std::string uri_)
        : uri(std::move(uri_)) {}
    WarningMessage(const WarningMessage&) = default;
    WarningMessage(WarningMessage&&) = delete;
    WarningMessage& operator=(const WarningMessage&) = default;
    WarningMessage& operator=(WarningMessage&&) = delete;
    virtual std::string msg() const = 0;
    virtual ~WarningMessage() = default;
    virtual morphio::enums::Warning warning() const = 0;

    std::string uri;
};

struct ZeroDiameter: public WarningMessage {
    ZeroDiameter(std::string uri_, uint64_t lineNumber_)
        : WarningMessage(std::move(uri_))
        , lineNumber(lineNumber_) {}
    morphio::enums::Warning warning() const final {
        return Warning::ZERO_DIAMETER;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: zero diameter in file";
        return "\n" + details::errorLink(uri, lineNumber, errorLevel) + description;
    }

    uint64_t lineNumber;
};

struct SectionTypeChanged: public WarningMessage {
    SectionTypeChanged(std::string uri_, uint64_t lineNumber_)
        : WarningMessage(std::move(uri_))
        , lineNumber(lineNumber_) {}
    morphio::enums::Warning warning() const final {
        return Warning::SECTION_TYPE_CHANGED;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description =
            "Warning: Type changed within section, without bifurcation";
        return "\n" + details::errorLink(uri, lineNumber, errorLevel) + description;
    }

    uint64_t lineNumber;
};

struct DisconnectedNeurite: public WarningMessage {
    DisconnectedNeurite(std::string uri_, uint64_t lineNumber_)
        : WarningMessage(std::move(uri_))
        , lineNumber(lineNumber_) {}
    Warning warning() const final {
        return Warning::DISCONNECTED_NEURITE;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description =
            "Warning: found a disconnected neurite.\n"
            "Neurites are not supposed to have parentId: -1\n"
            "(although this is normal if this neuron has no soma)";
        return "\n" + details::errorLink(uri, lineNumber, errorLevel) + description;
    }

    uint64_t lineNumber;
};

struct NoSomaFound: public WarningMessage {
    explicit NoSomaFound(std::string uri_)
        : WarningMessage(std::move(uri_)) {}
    Warning warning() const final {
        return Warning::NO_SOMA_FOUND;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: no soma found in file";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

struct SomaNonConform: public WarningMessage {
    explicit SomaNonConform(std::string uri_, std::string description_)
        : WarningMessage(std::move(uri_))
        , description(std::move(description_)) {}
    Warning warning() const final {
        return Warning::SOMA_NON_CONFORM;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
    std::string description;
};

struct WrongRootPoint: public WarningMessage {
    explicit WrongRootPoint(std::string uri_, std::vector<unsigned int> lineNumbers_)
        : WarningMessage(std::move(uri_))
        , lineNumbers(std::move(lineNumbers_)) {}
    Warning warning() const final {
        return Warning::WRONG_ROOT_POINT;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        std::ostringstream oss;
        oss << "Warning: with a 3 points soma, neurites must be connected to the first soma point:";
        for (const auto& lineNumber : lineNumbers) {
            oss << "\n" + details::errorLink(uri, lineNumber, errorLevel);
        }
        return oss.str();
    }
    std::vector<unsigned int> lineNumbers;
};

struct AppendingEmptySection: public WarningMessage {
    explicit AppendingEmptySection(std::string uri_, uint32_t sectionId_)
        : WarningMessage(std::move(uri_))
        , sectionId(sectionId_) {}
    Warning warning() const final {
        return Warning::APPENDING_EMPTY_SECTION;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: appending empty section with id: ";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description +
               std::to_string(sectionId);
    }
    uint32_t sectionId;
};

struct WrongDuplicate: public WarningMessage {
    explicit WrongDuplicate(std::string uri_,
                            std::shared_ptr<morphio::mut::Section> current_,
                            std::shared_ptr<morphio::mut::Section> parent_)
        : WarningMessage(std::move(uri_))
        , current(std::move(current_))
        , parent(std::move(parent_)) {}
    std::string msg() const final;

    Warning warning() const final {
        return Warning::WRONG_DUPLICATE;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::shared_ptr<morphio::mut::Section> current;
    std::shared_ptr<morphio::mut::Section> parent;
};

struct OnlyChild: public WarningMessage {
    explicit OnlyChild(std::string uri_, unsigned int parentId_, unsigned int childId_)
        : WarningMessage(std::move(uri_))
        , parentId(parentId_)
        , childId(childId_) {}
    Warning warning() const final {
        return Warning::ONLY_CHILD;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        std::ostringstream oss;
        oss << "Warning: section " << childId << " is the only child of "
            << "section: " << std::to_string(parentId)
            << "\nIt will be merged with the parent section";

        return "\n" + details::errorLink(uri, 0, errorLevel) + oss.str();
    }
    unsigned int parentId;
    unsigned int childId;
};

struct WriteNoSoma: public WarningMessage {
    WriteNoSoma()
        : WarningMessage(std::string()) {}
    Warning warning() const final {
        return Warning::WRITE_NO_SOMA;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: writing file without a soma";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

struct WriteEmptyMorphology: public WarningMessage {
    WriteEmptyMorphology()
        : WarningMessage(std::string()) {}
    Warning warning() const final {
        return Warning::WRITE_EMPTY_MORPHOLOGY;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description =
            "Warning: Skipping an attempt to write an empty morphology.";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

struct WriteUndefinedSoma: public WarningMessage {
    WriteUndefinedSoma()
        : WarningMessage(std::string()) {}
    Warning warning() const final {
        return Warning::WRITE_UNDEFINED_SOMA;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description = "Warning: writing soma set to SOMA_UNDEFINED";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

struct MitochondriaWriteNotSupported: public WarningMessage {
    MitochondriaWriteNotSupported()
        : WarningMessage(std::string()) {}
    Warning warning() const final {
        return Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description =
            "Warning: this cell has mitochondria, they cannot be saved in "
            " ASC or SWC format. Please use H5 if you want to save them.";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

struct SomaNonContour: public WarningMessage {
    SomaNonContour()
        : WarningMessage(std::string()) {}
    Warning warning() const final {
        return Warning::SOMA_NON_CONTOUR;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description =
            "Soma must be a contour for ASC and H5: see "
            "https://github.com/BlueBrain/MorphIO/issues/457";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

struct SomaNonCylinderOrPoint: public WarningMessage {
    SomaNonCylinderOrPoint()
        : WarningMessage(std::string()) {}
    Warning warning() const final {
        return Warning::SOMA_NON_CYLINDER_OR_POINT;
    }
    morphio::readers::ErrorLevel errorLevel = morphio::readers::ErrorLevel::WARNING;
    std::string msg() const final {
        static const char* description =
            "Soma must be stacked cylinders or a point: see "
            "https://github.com/BlueBrain/MorphIO/issues/457";
        return "\n" + details::errorLink(uri, 0, errorLevel) + description;
    }
};

class WarningHandler
{
  public:
    WarningHandler() = default;
    WarningHandler(WarningHandler&&) = default;
    WarningHandler& operator=(const WarningHandler&) = default;
    WarningHandler& operator=(WarningHandler&&) = default;
    WarningHandler(WarningHandler&) = default;
    virtual ~WarningHandler() = default;

    virtual void emit(std::shared_ptr<WarningMessage>) = 0;
    void setIgnoredWarning(enums::Warning warning, bool ignore);
    bool isIgnored(enums::Warning warning);

    // To maintain backwards compatibility, these exist, eventhough they aren't applicable to things
    // like the `WarningHandlerCollector` since one can post-process the errors, and raise
    // exceptions oneself.
    virtual int getMaxWarningCount() const = 0;
    virtual void setMaxWarningCount(int warningCount) = 0;
    virtual bool getRaiseWarnings() const = 0;
    virtual void setRaiseWarnings(bool raise) = 0;

  private:
    std::set<enums::Warning> ignoredWarnings_;
};

/// This warning handler prints warnings immediately to STDERR
class WarningHandlerPrinter: public WarningHandler
{
  public:
    int getMaxWarningCount() const final;
    void setMaxWarningCount(int warningCount) final;
    bool getRaiseWarnings() const final;
    void setRaiseWarnings(bool raise) final;
    void emit(std::shared_ptr<morphio::WarningMessage> wm) final;

  private:
    uint32_t errorCount = 0;
    int32_t maxWarningCount_ = 100;
    bool raiseWarnings_ = false;
};

/// This warning handler collects the warnings, which can be retrieved with `getAll()`
class WarningHandlerCollector: public WarningHandler
{
  public:
    struct Emission {
        Emission(bool wasMarkedIgnore_, std::shared_ptr<WarningMessage> warning_)
            : wasMarkedIgnore(wasMarkedIgnore_)
            , warning(std::move(warning_)) {}
        bool wasMarkedIgnore = false;
        std::shared_ptr<WarningMessage> warning;
    };

    int getMaxWarningCount() const final;
    void setMaxWarningCount(int warningCount) final;
    bool getRaiseWarnings() const final;
    void setRaiseWarnings(bool raise) final;
    void emit(std::shared_ptr<WarningMessage> wm) final;
    void reset();
    std::vector<Emission> getAll() const;

  private:
    std::vector<Emission> m;
};

}  // namespace morphio
