/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>     // std::shared_ptr
#include <vector>     // std::vector

#include <morphio/enums.h>        // Warning, Option
#include <morphio/error_warning_handling.h> // ErrorAndWarningHandler
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

namespace readers {

/** Level of error reporting **/
enum ErrorLevel {
    INFO,     //!< Info
    WARNING,  //!< Warning
    ERROR     //!< Error
};
}  // namespace readers

std::shared_ptr<ErrorAndWarningHandler> getErrorHandler();

}  // namespace morphio
