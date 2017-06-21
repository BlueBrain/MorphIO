/*
 * Copyright (C) 2017 Tristan Carel <tristan.carel@epfl.ch>
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

#ifndef MORPHO_TOOL_MORPHO_READER_HPP
#define MORPHO_TOOL_MORPHO_READER_HPP

#include <string>

#include <morpho/morpho_tree.hpp>

namespace morpho {
namespace reader {

/**
 * Load morpho tree from file, in either h5 or swc format.
 *
 * @param file Path to the file to load. File's extension matters.
 * @return Memory representation of the tree
 */
morpho_tree create_morpho_tree(const std::string &file);

} // namespace reader
} // namespace morpho

#endif // MORPHO_TOOL_MORPHO_READER_HPP
