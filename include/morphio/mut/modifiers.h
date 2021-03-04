#pragma once

#include <morphio/types.h>

namespace morphio {
namespace mut {
namespace modifiers {
/**
   Only the first and last points of each sections are kept
**/
template <typename Type>
void two_points_sections(morphio::mut::TMorphology<Type>& morpho);

/**
   Remove duplicated points
**/
template <typename Type>
void no_duplicate_point(morphio::mut::TMorphology<Type>& morpho);

/**
   Reduce the soma to a sphere placed at the center of gravity of soma points
   and whose radius is the averaged distance between the soma points and the
center of gravity
**/
template <typename Type>
void soma_sphere(morphio::mut::TMorphology<Type>& morpho);

template <typename Type>
void nrn_order(morphio::mut::TMorphology<Type>& morpho);

}  // namespace modifiers

}  // namespace mut

}  // namespace morphio
