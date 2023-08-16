/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>
#include <cmath>
#include <morphio/mut/modifiers.h>
#include <morphio/mut/morphology.h>

namespace morphio {
namespace mut {
namespace modifiers {

void two_points_sections(morphio::mut::Morphology& morpho) {
    for (auto it = morpho.depth_begin(); it != morpho.depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        size_t size = section->points().size();
        if (size < 2) {
            continue;
        }
        section->points() = {section->points()[0], section->points()[size - 1]};
        section->diameters() = {section->diameters()[0], section->diameters()[size - 1]};
        if (!section->perimeters().empty()) {
            section->perimeters() = {section->perimeters()[0], section->perimeters()[size - 1]};
        }
    }
}

void no_duplicate_point(morphio::mut::Morphology& morpho) {
    for (auto it = morpho.depth_begin(); it != morpho.depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        size_t size = section->points().size();

        if (size < 1 || (*it)->isRoot()) {
            continue;
        }

        section->points().erase(section->points().begin());
        section->diameters().erase(section->diameters().begin());

        if (!section->perimeters().empty()) {
            section->perimeters().erase(section->perimeters().begin());
        }
    }
}

void soma_sphere(morphio::mut::Morphology& morpho) {
    auto soma = morpho.soma();
    const auto size = static_cast<morphio::floatType>(soma->points().size());

    if (size < 2) {
        return;
    }

    floatType x = 0;
    floatType y = 0;
    floatType z = 0;
    floatType r = 0;

    for (const Point& point : soma->points()) {
        x += point[0] / size;
        y += point[1] / size;
        z += point[2] / size;
    }

    for (auto point : soma->points()) {
#ifdef MORPHIO_USE_DOUBLE
        r += sqrt(pow(point[0] - x, 2) + pow(point[1] - y, 2) + pow(point[2] - z, 2)) / size;
#else
        r += sqrtf(powf(point[0] - x, 2) + powf(point[1] - y, 2) + powf(point[2] - z, 2)) / size;
#endif
    }

    soma->points() = {{x, y, z}};
    soma->diameters() = {r};
}

static bool NRN_order_comparator(std::shared_ptr<Section> a, std::shared_ptr<Section> b) {
    return a->type() < b->type();
}

void nrn_order(morphio::mut::Morphology& morpho) {
    std::stable_sort(morpho._rootSections.begin(),
                     morpho._rootSections.end(),
                     NRN_order_comparator);
}

}  // namespace modifiers

}  // namespace mut

}  // namespace morphio
