/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "morphio/warning_handling.h"
#include <morphio/types.h>

namespace morphio {
namespace readers {
namespace asc {
Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options,
                          WarningHandler*);
}  // namespace asc
}  // namespace readers
}  // namespace morphio
