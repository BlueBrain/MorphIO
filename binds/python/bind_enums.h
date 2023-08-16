/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <pybind11/pybind11.h>

enum IterType { DEPTH_FIRST, BREADTH_FIRST, UPSTREAM };

void bind_enums(pybind11::module&);
