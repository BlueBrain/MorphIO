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
bool hasPerimeterData(const morphio::mut::Morphology& morpho);
std::string version_string();

}  // namespace details
}  // namespace writer
}  // namespace mut
}  // namespace morphio
