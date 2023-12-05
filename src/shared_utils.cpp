/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
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

}  // namespace morphio
