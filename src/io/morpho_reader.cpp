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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <morpho/morpho_reader.hpp>

#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_swc.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <hadoken/format/format.hpp>

namespace morpho {
namespace reader {

morpho_tree create_morpho_tree(const std::string& file) {
    using boost::algorithm::ends_with;
    using hadoken::format::scat;
    if (ends_with(file, ".h5")) {
        morpho::h5_v1::morpho_reader reader(file);
        return reader.create_morpho_tree();
    } else if (ends_with(file, ".swc")) {
        morpho::swc_v1::morpho_reader reader(file);
        return reader.create_morpho_tree();
    } else {
        throw std::logic_error(scat("Unsupported morphology file type: ", file));
    }
}

} // namespace reader
} // namespace morpho
