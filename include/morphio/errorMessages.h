/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>     // std::shared_ptr
#include <vector>     // std::vector

#include <morphio/enums.h>             // Warning
#include <morphio/warning_handling.h>  // ErrorAndWarningHandler

namespace morphio {

/** Set the maximum number of warnings to be printed; -1 for unlimited **/
void set_maximum_warnings(int n_warnings);
/** Set whether to interpet warning as errors **/
void set_raise_warnings(bool is_raise);
/** Set a warning to ignore **/
void set_ignored_warning(enums::Warning warning, bool ignore = true);
/** Set an array of warnings to ignore **/
void set_ignored_warning(const std::vector<enums::Warning>& warning, bool ignore = true);

std::shared_ptr<WarningHandler> getWarningHandler();

}  // namespace morphio
