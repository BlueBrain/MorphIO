/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <bitset>
#include <cstdint>

#include "shared_utils.hpp"

#include <ghc/filesystem.hpp>

namespace morphio {

floatType _somaSurface(const SomaType type,
                       const range<const floatType>& diameters,
                       const range<const Point>& points) {
    size_t size = points.size();

    switch (type) {
    case SOMA_SINGLE_POINT: {
        if (diameters.size() != 1) {
            throw MorphioError(readers::ErrorMessages().ERROR_SOMA_INVALID_SINGLE_POINT());
        }
        floatType radius = diameters[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }
    case SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS: {
        if (diameters.size() != 3) {
            throw MorphioError(readers::ErrorMessages().ERROR_SOMA_INVALID_THREE_POINT_CYLINDER());
        }
        floatType radius = diameters[0] / 2;
        return 4 * morphio::PI * radius * radius;
    }
    case SOMA_CYLINDERS: {
        // Surface is approximated as the sum of areas of the conical frustums
        // defined by each segments. Does not include the endcaps areas
        floatType surface = 0;
        for (unsigned int i = 0; i < size - 1; ++i) {
            floatType r0 = static_cast<morphio::floatType>(diameters[i]) / 2;
            floatType r1 = static_cast<morphio::floatType>(diameters[i + 1]) / 2;
            floatType h2 = euclidean_distance(points[i], points[i + 1]);
            auto s = morphio::PI * (r0 + r1) * std::sqrt((r0 - r1) * (r0 - r1) + h2 * h2);
            surface += s;
        }
        return surface;
    }
    case SOMA_SIMPLE_CONTOUR: {
        throw NotImplementedError("Surface is not implemented for SOMA_SIMPLE_CONTOUR");
    }
    case SOMA_UNDEFINED:
    default: {
        morphio::readers::ErrorMessages err;
        throw SomaError(err.ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA("Soma::surface"));
    }
    }
}


bool is_directory(const std::string& path) {
    return ghc::filesystem::exists(path) &&
           ghc::filesystem::is_directory(ghc::filesystem::canonical(path));
}

bool is_regular_file(const std::string& path) {
    return ghc::filesystem::exists(path) &&
           ghc::filesystem::is_regular_file(ghc::filesystem::canonical(path));
}

std::string join_path(const std::string& dirname, const std::string& filename) {
    return (ghc::filesystem::path(dirname) / filename).string();
}
namespace details {
ThreePointSomaStatus checkNeuroMorphoSoma(const std::array<Point, 3>& points, floatType radius) {
    //  NeuroMorpho is the main provider of morphologies, but they
    //  with SWC as their default file format: they convert all
    //  uploads to SWC.  In the process of conversion, they turn all
    //  somas into their custom 'Three-point soma representation':
    //  http://neuromorpho.org/SomaFormat.html
    //
    //  If the 2nd and the 3rd point have the same x,z,d values then the only valid soma is:
    //  1 1 x   y   z r -1
    //  2 1 x (y-r) z r  1
    //  3 1 x (y+r) z r  1
    //
    //  However, Prof Ascoli responded that they are relaxing the format:
    //  "I think that any 3 point soma would do as long as it represents
    //  a symmetric cylinder, and all trees stem from the central one of the 3 points."
    //  https://github.com/BlueBrain/morph-tool/issues/117#issuecomment-1772263991
    //
    //  Which I interpret to mean that as long as the `r` offset is applied along the same
    //  dimension, it's fine; and the +/- can either happen on point 2 or 3:
    //  EX, this is ok:
    //  1 1 x y  z      r -1
    //  2 1 x y (z + r) r  1 <- have `+` first
    //  3 1 x y (z - r) r  1

    auto withinEpsilon = [](floatType a, floatType b) {
        return std::fabs(a - b) < morphio::epsilon;
    };

    std::bitset<3> column_mask = {};
    for (uint8_t i = 0; i < 3; ++i) {
        column_mask[i] = (withinEpsilon(points[0][i], points[1][i]) &&
                          withinEpsilon(points[0][i], points[2][i]));
    }

    if (column_mask.none()) {
        return ZeroColumnsAreTheSame;
    } else if (column_mask.count() == 1) {
        return OneColumnIsTheSame;
    } else if (column_mask.all()) {
        return ThreeColumnsAreTheSame;
    }

    const size_t col = !column_mask[0] ? 0 : !column_mask[1] ? 1 : 2;

    if (!(withinEpsilon(points[0][col], points[1][col] - radius) &&
          withinEpsilon(points[0][col], points[2][col] + radius)) &&
        !(withinEpsilon(points[0][col], points[1][col] + radius) &&
          withinEpsilon(points[0][col], points[2][col] - radius))) {
        return NotRadiusOffset;
    }

    return Conforms;
}

std::ostream& operator<<(std::ostream& os, ThreePointSomaStatus s) {
    switch (s) {
    case ZeroColumnsAreTheSame:
        os << "None of the columns (ie: all the X, Y or Z values) are the same.";
        break;
    case OneColumnIsTheSame:
        os << "Only one column has the same coordinates.";
        break;
    case ThreeColumnsAreTheSame:
        os << "All three columns have the same coordinates.";
        break;
    case NotRadiusOffset:
        os << "The non-constant columns is not offset by +/- the radius from the initial sample.";
        break;
    case Conforms:
        os << "Three point soma conforms";
        break;
    }
    return os;
}

}  // namespace details
}  // namespace morphio
