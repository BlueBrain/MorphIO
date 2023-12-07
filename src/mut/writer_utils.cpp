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

bool hasPerimeterData(const morphio::mut::Morphology& morph) {
    return !morph.rootSections().empty() && !morph.rootSections().front()->perimeters().empty();
}

std::string version_string() {
    return std::string("Created by MorphIO v") + getVersionString();
}

bool emptyMorphology(const morphio::mut::Morphology& morph) {
    if (morph.soma()->points().empty() && morph.rootSections().empty()) {
        printError(Warning::WRITE_EMPTY_MORPHOLOGY,
                   readers::ErrorMessages().WARNING_WRITE_EMPTY_MORPHOLOGY());
        return true;
    }
    return false;
}

void validateContourSoma(const morphio::mut::Morphology& morph) {
    const std::shared_ptr<Soma>& soma = morph.soma();
    const std::vector<Point>& somaPoints = soma->points();

    if (somaPoints.empty()) {
        printError(Warning::WRITE_NO_SOMA, readers::ErrorMessages().WARNING_WRITE_NO_SOMA());
    } else if (soma->type() == SOMA_UNDEFINED) {
        printError(Warning::WRITE_UNDEFINED_SOMA,
                   readers::ErrorMessages().WARNING_UNDEFINED_SOMA());
    } else if (soma->type() != SomaType::SOMA_SIMPLE_CONTOUR) {
        printError(Warning::SOMA_NON_CONTOUR, readers::ErrorMessages().WARNING_SOMA_NON_CONTOUR());
    } else if (somaPoints.size() < 3) {
        throw WriterError(readers::ErrorMessages().ERROR_SOMA_INVALID_CONTOUR());
    }
}

void validateHasNoPerimeterData(const morphio::mut::Morphology& morph) {
    if (details::hasPerimeterData(morph)) {
        throw WriterError(readers::ErrorMessages().ERROR_PERIMETER_DATA_NOT_WRITABLE());
    }
}

void validateHasNoMitochondria(const morphio::mut::Morphology& morph) {
    if (!morph.mitochondria().rootSections().empty()) {
        printError(Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED,
                   readers::ErrorMessages().WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED());
    }
}

}  // namespace details
}  // namespace writer
}  // namespace mut
}  // namespace morphio
