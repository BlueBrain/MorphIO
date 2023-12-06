/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/errorMessages.h>

#include "writer_utils.h"

namespace morphio {
namespace mut {
namespace writer {
namespace details {


void checkSomaHasSameNumberPointsDiameters(const Soma& soma) {
    const size_t n_points = soma.points().size();
    const size_t n_diameters = soma.diameters().size();

    if (n_points != n_diameters) {
        throw morphio::WriterError(morphio::readers::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
            "soma points", n_points, "soma diameters", n_diameters));
    }
}

bool hasPerimeterData(const morphio::mut::Morphology& morpho) {
    return !morpho.rootSections().empty() && !morpho.rootSections().front()->perimeters().empty();
}

std::string version_string() {
    return std::string("Created by MorphIO v") + getVersionString();
}


}  // namespace details
}  // namespace writer
}  // namespace mut
}  // namespace morphio
