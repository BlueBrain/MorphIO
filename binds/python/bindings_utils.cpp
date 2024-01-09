/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bindings_utils.h"

#include <pybind11/pybind11.h>

#include <pybind11/numpy.h>  // py::array_t

#include <array>
#include <string>

#include <morphio/exceptions.h>  // morphio::MorphioError
#include <morphio/types.h>

namespace py = pybind11;

namespace {

void _raise_if_wrong_shape(const py::buffer_info& info) {
    const auto& shape = info.shape;
    if (shape.size() != 2 || info.shape[1] != 3) {
        std::string shape_str;
        for (unsigned int i = 0; i < shape.size(); ++i) {
            shape_str += std::to_string(shape[i]);
            if (i != shape.size() - 1) {
                shape_str += ", ";
            }
        }
        throw morphio::MorphioError("Wrong array shape. Expected: (X, 3), got: (" + shape_str +
                                    ")");
    }
}
}  // anonymous namespace

morphio::Points array_to_points(const py::array_t<morphio::floatType>& buf) {
    py::buffer_info info = buf.request();
    _raise_if_wrong_shape(info);
    morphio::Points points;
    points.reserve(static_cast<size_t>(info.shape[0]));

    for (int i = 0; i < info.shape[0]; ++i) {
        points.push_back({*buf.data(i, 0), *buf.data(i, 1), *buf.data(i, 2)});
    }
    return points;
}

py::array_t<morphio::floatType> span_array_to_ndarray(
    const morphio::range<const std::array<morphio::floatType, 3>>& span) {
    const void* ptr = static_cast<const void*>(span.data());
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        const_cast<void*>(ptr),                              /* Pointer to buffer */
        sizeof(morphio::floatType),                          /* Size of one scalar */
        py::format_descriptor<morphio::floatType>::format(), /* Python struct-style format
                                                                descriptor */
        2,                                                   /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        {static_cast<int>(span.size()), 3}, /* buffer dimentions */
        {sizeof(morphio::floatType) * 3,    /* Strides (in bytes) for each index */
         sizeof(morphio::floatType)});
    return py::array(buffer_info);
}
