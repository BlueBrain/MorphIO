/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iostream>  // std::cerr
#include <string>
#include <vector>

#include <morphio/enums.h>
#include <morphio/errorMessages.h>
#include <morphio/error_warning_handling.h>
#include <morphio/exceptions.h>

namespace {

class StaticErrorAndWarningHandler: public morphio::ErrorAndWarningHandler
{
public:
    void emit(const morphio::enums::Warning& warning, const std::string& msg) final {
        const int maxWarningCount = getMaxWarningCount();

        if (isIgnored(warning) || maxWarningCount == 0) {
            return;
        }

        if (getRaiseWarnings()) {
            throw morphio::MorphioError(msg);
        }

        if (maxWarningCount < 0 || errorCount <= maxWarningCount) {
            std::cerr << msg << '\n';
            if (errorCount == maxWarningCount) {
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
private:
    int errorCount = 0;
};

} // namespace

namespace morphio {
void ErrorAndWarningHandler::emit(const std::unique_ptr<WarningMessage> wm) {
        emit(wm->warning(), wm->msg());
    }

bool ErrorAndWarningHandler::isIgnored(enums::Warning warning) {
    return ignoredWarnings_.find(warning) != ignoredWarnings_.end();
}

void ErrorAndWarningHandler::setIgnoredWarning(enums::Warning warning, bool ignore) {
    if (ignore) {
        ignoredWarnings_.insert(warning);
    } else {
        ignoredWarnings_.erase(warning);
    }
}

int ErrorAndWarningHandler::getMaxWarningCount() const {
    return maxWarningCount_;
}

void ErrorAndWarningHandler::setMaxWarningCount(int warningCount) {
    maxWarningCount_ = warningCount;
}

bool ErrorAndWarningHandler::getRaiseWarnings() const {
    return raiseWarnings_;
}
void ErrorAndWarningHandler::setRaiseWarnings(bool raise) {
    raiseWarnings_ = raise;
}

std::shared_ptr<morphio::ErrorAndWarningHandler> getErrorHandler() {
    static StaticErrorAndWarningHandler error_handler;
    return {std::shared_ptr<StaticErrorAndWarningHandler>{}, &error_handler};
}

/*
 * Controls the maximum number of warning to be printed on screen.
 * 0 will print no warning
 * -1 will print them all
 */
void set_maximum_warnings(int n_warnings) {
    auto static_handler = getErrorHandler();
    static_handler->setMaxWarningCount(n_warnings);
}

/* Whether to raise warning as errors */
void set_raise_warnings(bool is_raise) {
    auto static_handler = getErrorHandler();
    static_handler->setRaiseWarnings(is_raise);
}

/** Ignore/Unignore a specific warning message */
void set_ignored_warning(enums::Warning warning, bool ignore) {
    auto static_handler = getErrorHandler();
    static_handler->setIgnoredWarning(warning, ignore);
}

/** Ignore/Unignore a specific warning message */
void set_ignored_warning(const std::vector<enums::Warning>& warnings, bool ignore) {
    for (auto warning : warnings) {
        set_ignored_warning(warning, ignore);
    }
}

namespace readers {
}  // namespace readers

}  // namespace morphio
