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
#ifndef MORPHO_TOOL_MORPHO_SERIALIZATION_HPP
#define MORPHO_TOOL_MORPHO_SERIALIZATION_HPP

#include <istream>
#include <ostream>

namespace morpho {

/// Forward declaration
class morpho_tree;

///
/// \brief Available serialization format supported
///
enum class serialization_format: unsigned char {
    /// compact binary representation
    BINARY = 0x01,
    /// compact binary representation portable over different architectures
    PORTABLE_BINARY = 0x02,
    JSON = 0x04,
    XML = 0x08
};


/**
 * \brief Serialize a morpho tree
 * @param tree  morpho tree to serialize
 * @param format serialization format
 * @return serialized data
 */
std::string serialize(const morpho_tree& tree,
                      const serialization_format& format);

///
/// \brief Serialize a morpho tree to an output stream
/// \param tree morpho tree to serialize
/// \param stream destination
/// \param format serialization format.
///
void serialize(const morpho_tree& tree, std::ostream& stream,
               const serialization_format& format);

///
/// \brief Instantiate a morpho tree from serialized data
/// \param stream input stream to read data from
/// \param format serialization format
/// \return new morpho tree initialized with given data
///
morpho_tree deserialize(std::istream& stream,
                        const serialization_format& format);

///
/// \brief Instantiate a morpho tree from serialized data
/// \param data serialized data
/// \param format serialization format
/// \return new morpho tree initialized with given data
///
morpho_tree deserialize(const std::string& data,
                        const serialization_format& format);

} // namespace morpho

#endif // MORPHO_TOOL_MORPHO_SERIALIZATION_HPP
