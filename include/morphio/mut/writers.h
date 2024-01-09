/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/warning_handling.h>

namespace morphio {
namespace mut {
namespace writer {

/** Save morphology in SWC format */
void swc(const Morphology& morphology,
         const std::string& filename,
         std::shared_ptr<WarningHandler> handler);

/** Save morphology in ASC format */
void asc(const Morphology& morphology,
         const std::string& filename,
         std::shared_ptr<WarningHandler> handler);

/** Save morphology in H5 format */
void h5(const Morphology& morphology,
        const std::string& filename,
        std::shared_ptr<WarningHandler> handler);

}  // namespace writer
}  // end namespace mut
}  // end namespace morphio
