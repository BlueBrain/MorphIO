/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/errorMessages.h>
#include <morphio/types.h>

#include "point_utils.h"


namespace morphio {

floatType _somaSurface(const SomaType type,
                       const range<const floatType>& diameters,
                       const range<const Point>& points);

template <typename T>
void _appendVector(std::vector<T>& to, const std::vector<T>& from, int offset) {
    to.insert(to.end(), from.begin() + offset, from.end());
}

template <typename T>
std::vector<typename T::Type> copySpan(const std::vector<typename T::Type>& data,
                                       SectionRange range) {
    if (data.empty()) {
        return {};
    }

    return {data.begin() + static_cast<long int>(range.first),
            data.begin() + static_cast<long int>(range.second)};
}

/**
 * Is `path` a directory?
 *
 * Symlinks to directories are considered directories.
 */
bool is_directory(const std::string& path);

/**
 * Is `path` a regular file?
 *
 * Symlinks to regular files are considered files.
 */
bool is_regular_file(const std::string& path);

/**
 * Join `dirname` and `filename` into one path.
 *
 * This follows the Python `os.path.join` semantics, i.e.,
 *   - join_path("", "foo") == "foo" (not "/foo")
 *   - join_path("/usr", "/home/foo") == "/home/foo" (not "/usr/home/foo")
 */
std::string join_path(const std::string& dirname, const std::string& filename);

namespace property {

template <typename T>
bool compare(const T& el1, const T& el2, const std::string& name, LogLevel logLevel) {
    if (el1 == el2) {
        return true;
    }

    if (logLevel > LogLevel::ERROR) {
        printError(Warning::UNDEFINED, name + " differs");
    }
    return false;
}
}  // namespace property

namespace details {
template <typename T>
bool checkNeuroMorphoSoma(std::array<Point, 3> points, T radius) {
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

    auto geometryConforms = [&](size_t col) {
        size_t col0 = 0;  // column that has the +/- radius
        size_t col1 = 1;  // column where all points are the same
        size_t col2 = 2;  // column where all points are the same
        switch (col) {
        case 0:
            break;
        case 1:
            col0 = 1;
            col1 = 0;
            col2 = 2;
            break;
        default:
            col0 = 1;
            col1 = 2;
            col2 = 0;
            break;
        }
        return (
            // all three points in the non-changing columns have to be the same
            (withinEpsilon(points[0][col1], points[1][col1]) &&
             withinEpsilon(points[0][col1], points[1][col1]) &&
             withinEpsilon(points[0][col2], points[1][col2]) &&
             withinEpsilon(points[0][col2], points[2][col2])) &&
            (((withinEpsilon(points[0][col0], points[1][col0] - radius) &&
               withinEpsilon(points[0][col0], points[2][col0] + radius)) ||
              (withinEpsilon(points[0][col0], points[1][col0] + radius) &&
               withinEpsilon(points[0][col0], points[2][col0] - radius)))));
    };
    return geometryConforms(0) || geometryConforms(1) || geometryConforms(2);
}
}  // namespace details
}  // namespace morphio
