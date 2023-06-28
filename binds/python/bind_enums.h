/**
   A place for enums that are only used in the bindings
**/
#pragma once

#include <pybind11/pybind11.h>

enum IterType { DEPTH_FIRST, BREADTH_FIRST, UPSTREAM };

void bind_enums(pybind11::module&);
