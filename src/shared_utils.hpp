/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iosfwd>  // std::ostream
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
enum ThreePointSomaStatus {
    Conforms,
    ZeroColumnsAreTheSame,
    OneColumnIsTheSame,
    ThreeColumnsAreTheSame,
    NotRadiusOffset,
};

ThreePointSomaStatus checkNeuroMorphoSoma(const std::array<Point, 3>&, floatType);
std::ostream& operator<<(std::ostream& os, ThreePointSomaStatus s);
}  // namespace details
}  // namespace morphio
