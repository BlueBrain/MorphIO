/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <morphio/mut/morphology.h>
#include <morphio/mut/soma.h>
#include <morphio/warning_handling.h>

namespace morphio {
namespace mut {
namespace writer {
namespace details {

void checkSomaHasSameNumberPointsDiameters(const morphio::mut::Soma&);
bool hasPerimeterData(const morphio::mut::Morphology&);
std::string version_string();
bool emptyMorphology(const morphio::mut::Morphology&,
                     std::shared_ptr<morphio::WarningHandler> handler);
void validateContourSoma(const morphio::mut::Morphology&,
                         std::shared_ptr<morphio::WarningHandler> handler);
void validateHasNoPerimeterData(const morphio::mut::Morphology&);
void validateHasNoMitochondria(const morphio::mut::Morphology&,
                               std::shared_ptr<morphio::WarningHandler> handler);
void validateRootPointsHaveTwoOrMorePoints(const morphio::mut::Morphology& morph);

}  // namespace details
}  // namespace writer
}  // namespace mut
}  // namespace morphio
