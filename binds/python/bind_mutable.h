/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <pybind11/pybind11.h>

#include <morphio/dendritic_spine.h>
#include <morphio/endoplasmic_reticulum.h>
#include <morphio/enums.h>
#include <morphio/glial_cell.h>
#include <morphio/mut/dendritic_spine.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/soma.h>
#include <morphio/types.h>

void bind_mutable(pybind11::module& m);
