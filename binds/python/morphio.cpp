/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <pybind11/pybind11.h>

#include "bind_enums.h"
#include "bind_immutable.h"
#include "bind_misc.h"
#include "bind_mutable.h"
#include "bind_vasculature.h"
#include "bind_warnings_exceptions.h"


namespace py = pybind11;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

PYBIND11_MODULE(_morphio, m) {
    bind_enums(m);
    bind_warnings_exceptions(m);
    bind_misc(m);

    bind_immutable(m);

    py::module mut_module = m.def_submodule("mut");
    bind_mutable(mut_module);

    py::module vasc_module = m.def_submodule("vasculature");
    bind_vasculature(vasc_module);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
