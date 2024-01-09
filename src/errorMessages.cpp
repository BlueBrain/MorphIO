/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <vector>

#include <morphio/enums.h>
#include <morphio/errorMessages.h>
#include <morphio/exceptions.h>
#include <morphio/warning_handling.h>


namespace morphio {
/*
 * Controls the maximum number of warning to be printed on screen.
 * 0 will print no warning
 * -1 will print them all
 */
void set_maximum_warnings(int32_t n_warnings) {
    auto static_handler = getWarningHandler();
    static_handler->setMaxWarningCount(n_warnings);
}

/* Whether to raise warning as errors */
void set_raise_warnings(bool is_raise) {
    auto static_handler = getWarningHandler();
    static_handler->setRaiseWarnings(is_raise);
}

/** Ignore/Unignore a specific warning message */
void set_ignored_warning(enums::Warning warning, bool ignore) {
    auto static_handler = getWarningHandler();
    static_handler->setIgnoredWarning(warning, ignore);
}

/** Ignore/Unignore a specific warning message */
void set_ignored_warning(const std::vector<enums::Warning>& warnings, bool ignore) {
    for (auto warning : warnings) {
        set_ignored_warning(warning, ignore);
    }
}

std::shared_ptr<morphio::WarningHandler> getWarningHandler() {
    static morphio::WarningHandlerPrinter warning_handler;
    return {std::shared_ptr<morphio::WarningHandler>{}, &warning_handler};
}

}  // namespace morphio
