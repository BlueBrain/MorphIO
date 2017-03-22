/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#ifndef BRION_DETAIL_SILENCEHDF5
#define BRION_DETAIL_SILENCEHDF5

#include <H5Epublic.h>

namespace brion
{
namespace detail
{
class SilenceHDF5
{
public:
    SilenceHDF5()
        : _client_data(0)
    {
        H5Eget_auto2(H5E_DEFAULT, &_func, &_client_data);
        H5Eset_auto2(H5E_DEFAULT, 0, 0);
    }

    ~SilenceHDF5() { H5Eset_auto2(H5E_DEFAULT, _func, _client_data); }
private:
    H5E_auto2_t _func;
    void* _client_data;
};
}
}

#endif
