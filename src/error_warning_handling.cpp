#include <sstream>  // std::ostringstream

#include <morphio/error_warning_handling.h>
#include <morphio/mut/section.h>

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
        case ErrorLevel::INFO:
            return "info";
        case ErrorLevel::WARNING:
            return "warning";
        case ErrorLevel::ERROR:
            return "error";
        }
        throw std::runtime_error("Unknown ErrorLevel");
    };

    auto COLOR = [](ErrorLevel el) {
        switch (el) {
        case ErrorLevel::INFO:
            return "\033[1;34m";
        case ErrorLevel::WARNING:
            return "\033[1;33m";
        case ErrorLevel::ERROR:
            return "\033[1;31m";
        }
        throw std::runtime_error("Unknown ErrorLevel");
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
}  // namespace morphio
