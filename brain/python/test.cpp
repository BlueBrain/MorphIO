/* Copyright (c) 2016, Juan Hernando <jhernando@fi.upm.es>
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

#ifndef BRION_USE_BBPTESTDATA
namespace brain { void export_test() {} }
#else

#include <boost/python.hpp>

#include "submodules.h"

#include <BBP/TestDatasets.h>

namespace brain
{

void export_test()
{
    boost::python::list configs;
    for( const auto& config : bbp::test::getBlueconfigs( ))
        configs.append( config );

    boost::python::scope test = brain::exportSubmodule( "test" );

    test.attr("blue_config") = bbp::test::getBlueconfig();
    test.attr("blue_configs") = configs;
    test.attr("circuit_config") = bbp::test::getCircuitconfig();
    test.attr("root_data_path") = std::string( BBP_TESTDATA );
}

}
#endif
