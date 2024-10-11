#include <sstream>  // std::ostringstream

#include <morphio/mut/section.h>
#include <morphio/warning_handling.h>
#include <stdexcept>

namespace morphio {
namespace details {
std::string errorLink(const std::string& uri,
                      long unsigned int lineNumber,
                      morphio::readers::ErrorLevel errorLevel) {
    using morphio::readers::ErrorLevel;
    if (uri.empty()) {
        return {};
    }

    const auto SEVERITY = [](ErrorLevel el) {
        switch (el) {
        case ErrorLevel::WARNING:
            return "warning";
        case ErrorLevel::ERROR:
            return "error";
        default:
            throw std::runtime_error("Unknown ErrorLevel");
        }
    };

    auto COLOR = [](ErrorLevel el) {
        switch (el) {
        case ErrorLevel::WARNING:
            return "\033[1;33m";
        case ErrorLevel::ERROR:
            return "\033[1;31m";
        default:
            throw std::runtime_error("Unknown ErrorLevel");
        }
    };

    const std::string COLOR_END("\033[0m");

    return COLOR(errorLevel) + uri + ":" + std::to_string(lineNumber) + ":" + SEVERITY(errorLevel) +
           COLOR_END + "\n";
}
}  // namespace details

std::string WrongDuplicate::msg() const {
    std::string msg("Warning: while appending section: " + std::to_string(current->id()) +
                    " to parent: " + std::to_string(parent->id()));

    if (parent->points().empty()) {
        return "\n" + details::errorLink(uri, 0, readers::ErrorLevel::WARNING) + msg +
               "\nThe parent section is empty.";
    }

    if (current->points().empty()) {
        return "\n" + details::errorLink(uri, 0, readers::ErrorLevel::WARNING) + msg +
               "\nThe current section has no points. It should at "
               "least contains "
               "parent section last point";
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

    return "\n" + details::errorLink(uri, 0, readers::ErrorLevel::WARNING) + oss.str();
}

bool WarningHandler::isIgnored(enums::Warning warning) {
    return ignoredWarnings_.find(warning) != ignoredWarnings_.end();
}

void WarningHandler::setIgnoredWarning(enums::Warning warning, bool ignore) {
    if (ignore) {
        ignoredWarnings_.insert(warning);
    } else {
        ignoredWarnings_.erase(warning);
    }
}

int32_t WarningHandlerPrinter::getMaxWarningCount() const {
    return maxWarningCount_;
}

void WarningHandlerPrinter::setMaxWarningCount(int32_t warningCount) {
    maxWarningCount_ = warningCount;
}

bool WarningHandlerPrinter::getRaiseWarnings() const {
    return raiseWarnings_;
}
void WarningHandlerPrinter::setRaiseWarnings(bool raise) {
    raiseWarnings_ = raise;
}

void WarningHandlerPrinter::emit(std::shared_ptr<morphio::WarningMessage> wm) {
    const int maxWarningCount = getMaxWarningCount();

    const auto& warning = wm->warning();

    if (isIgnored(warning) || maxWarningCount == 0) {
        return;
    }

    if (getRaiseWarnings()) {
        throw morphio::MorphioError(wm->msg());
    }

    if (maxWarningCount < 0 || errorCount <= static_cast<uint32_t>(maxWarningCount)) {
        std::cerr << wm->msg() << '\n';
        if (maxWarningCount > 0 && errorCount == static_cast<uint32_t>(maxWarningCount)) {
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

int WarningHandlerCollector::getMaxWarningCount() const {
    throw std::runtime_error("WarningHandlerCollector does not implement getMaxWarningCount");
}
void WarningHandlerCollector::setMaxWarningCount(int /*warningCount*/) {
    throw std::runtime_error("WarningHandlerCollector does not implement setMaxWarningCount");
}
bool WarningHandlerCollector::getRaiseWarnings() const {
    throw std::runtime_error("WarningHandlerCollector does not implement getRaiseWarnings");
}
void WarningHandlerCollector::setRaiseWarnings(bool /*raise*/) {
    throw std::runtime_error("WarningHandlerCollector does not implement setRaiseWarnings");
}

void WarningHandlerCollector::emit(std::shared_ptr<WarningMessage> wm) {
    m.emplace_back(isIgnored(wm->warning()), wm);
}

void WarningHandlerCollector::reset() {
    m.erase(m.begin());
}

std::vector<WarningHandlerCollector::Emission> WarningHandlerCollector::getAll() const {
    return m;
}


}  // namespace morphio
