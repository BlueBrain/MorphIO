#include <pybind11/pybind11.h>

#include "bind_immutable.h"
#include "bind_misc.h"
#include "bind_mutable.h"
#include "bind_vasculature.h"

#include "bindings_utils.h"


namespace py = pybind11;

PYBIND11_MODULE(_morphio, m) {
    bind_misc(m);
    _immutable_binding_classes all_immutable_binding_classes = bind_immutable_classes(m);

    py::module mut_module = m.def_submodule("mut");

    _mutable_binding_classes all_mutable_binding_classes = bind_mutable_classes(mut_module);

    bind_immutable_methods(m, all_immutable_binding_classes);
    bind_mutable_methods(mut_module, all_mutable_binding_classes);

    py::module vasc_module = m.def_submodule("vasculature");
    bind_vasculature(vasc_module);
}
