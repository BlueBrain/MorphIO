/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/errorMessages.h>

#include "../error_message_generation.h"
#include "morphio/warning_handling.h"
#include "writer_utils.h"

namespace morphio {
namespace mut {
namespace writer {
namespace details {

using morphio::details::ErrorMessages;

void checkSomaHasSameNumberPointsDiameters(const Soma& soma) {
    const size_t n_points = soma.points().size();
    const size_t n_diameters = soma.diameters().size();

    if (n_points != n_diameters) {
        throw morphio::WriterError(ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
            "soma points", n_points, "soma diameters", n_diameters));
    }
}

bool hasPerimeterData(const morphio::mut::Morphology& morph) {
    return !morph.rootSections().empty() && !morph.rootSections().front()->perimeters().empty();
}

std::string version_string() {
    return std::string("Created by MorphIO v") + getVersionString();
}

bool emptyMorphology(const morphio::mut::Morphology& morph,
                     std::shared_ptr<morphio::WarningHandler> handler) {
    if (morph.soma()->points().empty() && morph.rootSections().empty()) {
        handler->emit(std::make_shared<morphio::WriteEmptyMorphology>());
        return true;
    }
    return false;
}

void validateContourSoma(const morphio::mut::Morphology& morph,
                         std::shared_ptr<morphio::WarningHandler> handler) {
    const std::shared_ptr<Soma>& soma = morph.soma();
    const std::vector<Point>& somaPoints = soma->points();

    if (somaPoints.empty()) {
        handler->emit(std::make_shared<morphio::WriteNoSoma>());
    } else if (soma->type() == SOMA_UNDEFINED) {
        handler->emit(std::make_shared<morphio::WriteUndefinedSoma>());
    } else if (soma->type() != SomaType::SOMA_SIMPLE_CONTOUR) {
        handler->emit(std::make_shared<morphio::SomaNonContour>());
    } else if (somaPoints.size() < 3) {
        throw WriterError(ErrorMessages().ERROR_SOMA_INVALID_CONTOUR());
    }
}

void validateHasNoPerimeterData(const morphio::mut::Morphology& morph) {
    if (details::hasPerimeterData(morph)) {
        throw WriterError(ErrorMessages().ERROR_PERIMETER_DATA_NOT_WRITABLE());
    }
}

void validateHasNoMitochondria(const morphio::mut::Morphology& morph,
                               std::shared_ptr<morphio::WarningHandler> handler) {
    if (!morph.mitochondria().rootSections().empty()) {
        handler->emit(std::make_shared<morphio::MitochondriaWriteNotSupported>());
    }
}

void validateRootPointsHaveTwoOrMorePoints(const morphio::mut::Morphology& morph) {
    for (const auto& root : morph.rootSections()) {
        if (root->points().size() < 2) {
            throw morphio::RawDataError("Root sections must have at least 2 points");
        }
    }

}

}  // namespace details
}  // namespace writer
}  // namespace mut
}  // namespace morphio
