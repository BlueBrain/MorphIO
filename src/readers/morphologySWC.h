/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <morphio/errorMessages.h>
#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace readers {
namespace swc {
Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options,
                          std::shared_ptr<WarningHandler>& warning_handler);
}  // namespace swc
}  // namespace readers
}  // namespace morphio
