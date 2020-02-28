#include <pybind11/pybind11.h>

#include "bind_immutable.h"
#include "bind_misc.h"
#include "bind_mutable.h"
#include "bind_vasculature.h"

namespace py = pybind11;

PYBIND11_MODULE(_morphio, m) {
    bind_misc(m);
    bind_immutable_module(m);

    py::module mut_module = m.def_submodule("mut");
    bind_mutable_module(mut_module);

    py::module vasc_module = m.def_submodule("vasculature");
    bind_vasculature(vasc_module);
}
