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
#ifndef MORPHO_CIRCUIT_HPP
#define MORPHO_CIRCUIT_HPP

#ifndef H5_USE_BOOST
#define H5_USE_BOOST
#endif

#include <string>
#include <utility>

#include "morpho_tree.hpp"
#include "morpho_types.hpp"

namespace morpho {

namespace circuit {

class circuit_reader {
public:
  circuit_reader(const std::string &filename_mvd3,
                 const std::string &morpho_directory);

  std::vector<morpho_tree> create_all_morpho_tree() const;

private:
  std::string _filename, _morpho_directory;
};

} // circuit

} // morpho

#endif // MORPHO_CIRCUIT_HPP
