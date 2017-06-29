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
#ifndef MORPHO_MESHER_HPP
#define MORPHO_MESHER_HPP

#include <bitset>
#include <morpho/morpho_tree.hpp>

namespace morpho {

///
/// \brief create mesh out of morphology
///
class morpho_mesher {
  public:
    enum mesh_tag {
        mesh_optimisation = 0,
        only_surface = 1,
        force_manifold = 2,

    };

    ///
    /// \brief construct a Mesher object from a morphology tree
    /// \param tree morphology tree to use
    /// \param output_mesh_file filepath of the mesh to export
    ///
    morpho_mesher(const std::shared_ptr<morpho_tree>& tree,
                  const std::string& output_mesh_file);

    ///
    /// \brief enable or disable an option for the meshing process
    /// \param value
    ///
    void set_mesh_tag(mesh_tag, bool value);

    ///
    /// \brief set the error bound value for the surface's dichotomy search (
    /// higher is more precise )
    /// \param inv_error
    ///
    void set_error_bound(double inv_error);

    ///
    /// \brief set the facet size of the mesh ( delaunay ball radius )
    ///
    void set_face_size(double face_size);

    ///
    /// \brief start meshing process
    ///
    void execute();

  private:
    std::bitset<64> _flags;
    double _error_bound, _facet_size;

    std::string _output_mesh_file;
    std::shared_ptr<morpho_tree> _tree;

    void execute_3d_meshing();
    void execute_surface_meshing();
    void log_parameters();
};
}

#endif // MORPHO_MESHER_HPP
