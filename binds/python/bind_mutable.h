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

#include "bindings_utils.h"


mutable_binding_classes bind_mutable_classes(pybind11::module& m);

void bind_mutable_methods(mutable_binding_classes& mutable_classes);
