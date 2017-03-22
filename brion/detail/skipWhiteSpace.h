/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef BRION_DETAIL_SKIPWHITESPACE
#define BRION_DETAIL_SKIPWHITESPACE

#include <iostream>

namespace brion
{
namespace detail
{
struct SkipWhiteSpace
{
    explicit SkipWhiteSpace(size_t& line_count)
        : _line_count(&line_count)
    {
    }

    std::istream& operator()(std::istream& in) const
    {
        while (isspace(in.peek()))
        {
            if (in.get() == '\n')
            {
                ++(*_line_count);
            }
        }
        return in;
    }

    size_t* const _line_count;
};

inline std::istream& operator>>(std::istream& in, const SkipWhiteSpace& ws)
{
    return ws(in);
}
}
}
#endif
