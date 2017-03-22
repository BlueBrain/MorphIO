#pragma once

#include <numpy/_numpyconfig.h>
#if NPY_API_VERSION >= 0x00000007
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#endif
#include <numpy/arrayobject.h>
