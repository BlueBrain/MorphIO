/*
 * Copyright (C) 2015 Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */
#ifndef MORPHO_TRANSFORM_FILTERS_HPP
#define MORPHO_TRANSFORM_FILTERS_HPP

#include <morpho/morpho_transform.hpp>
#include <morpho/morpho_tree.hpp>

namespace morpho {

class morpho_operation;

///
/// \brief  delete_duplicate_point_operation
///
/// transform operations : Remove any consecutive duplicated point in every
/// section
///  of a given morpho-tree
///
class delete_duplicate_point_operation : public morpho_operation {
  public:
    delete_duplicate_point_operation();

    morpho_tree apply(const morpho_tree& tree) override;

    std::string name() const override;
};

///
/// \brief  duplicate_first_point_operation
///
/// transform operations : duplicate the last point of every section as
/// first point of the next section
///
class duplicate_first_point_operation : public morpho_operation {
  public:
    duplicate_first_point_operation();

    morpho_tree apply(const morpho_tree& tree) override;

    std::string name() const override;
};

///
/// \brief  soma_sphere
///
/// transform operations : transform a morphology with a soma based on a
/// line-loop into a morphology
///  with a soma modelized as a sphere.
///
///  The sphere is computed from the gravity center of the lineloop and from the
///  average distance between the center
///  and the points of the lineloop
///
///
class soma_sphere_operation : public morpho_operation {
  public:
    soma_sphere_operation();

    morpho_tree apply(const morpho_tree& tree) override;

    std::string name() const override;
};

///
/// \brief  simplify_section_extreme
///
/// transform operations : simplify all morphology section to the extreme by
/// keeping only first and last point
///
///
///
class simplify_section_extreme_operation : public morpho_operation {
  public:
    simplify_section_extreme_operation();

    morpho_tree apply(const morpho_tree& tree) override;

    std::string name() const override;
};

///
/// \brief  simplify_section_extreme
///
/// transform operations : transpose and rotate the morphology using a vector
/// and a quaternion
///
///
///
class transpose_operation : public morpho_operation {
  public:
    typedef std::array<double, 3> vector3d;
    typedef std::array<double, 4> quaternion3d;

    transpose_operation(const vector3d& vector_transpose,
                        const quaternion3d& quaternion_transpose);

    morpho_tree apply(const morpho_tree& tree) override;

    std::string name() const override;

  private:
    vector3d _trans;
    quaternion3d _rotate;
};

} // morpho

#endif // MORPHO_TRANSFORM_FILTERS_HPP
