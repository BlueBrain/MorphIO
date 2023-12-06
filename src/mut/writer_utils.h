/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <morphio/mut/morphology.h>
#include <morphio/mut/soma.h>

namespace morphio {
namespace mut {
namespace writer {
namespace details {

void checkSomaHasSameNumberPointsDiameters(const morphio::mut::Soma&);
bool hasPerimeterData(const morphio::mut::Morphology&);
std::string version_string();
bool emptyMorphology(const morphio::mut::Morphology&);
void validateContourSoma(const morphio::mut::Morphology&);
void validateHasNoPerimeterData(const morphio::mut::Morphology&);
void validateHasNoMitochondria(const morphio::mut::Morphology&);

}  // namespace details
}  // namespace writer
}  // namespace mut
}  // namespace morphio
