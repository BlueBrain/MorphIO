
/* Copyright (c) 2006-2016, Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <boost/python.hpp>

#include <brain/types.h>
#include <brain/spikeReportReader.h>
#include <brain/spikes.h>

using namespace brain;
using namespace boost::python;

namespace
{

SpikeReportReaderPtr initURI( const std::string& uri )
{
    return SpikeReportReaderPtr( new SpikeReportReader( brion::URI( uri )));
}

}

void export_SpikeReportReader()
{

class_< SpikeReportReader, boost::noncopyable >(
    "SpikeReportReader", no_init )
    .def( "__init__", make_constructor( initURI ))
    .def( "close", &SpikeReportReader::close )
    .def( "getStartTime", &SpikeReportReader::getStartTime )
    .def( "getEndTime", &SpikeReportReader::getEndTime )
    .def( "getSpikes", ( Spikes (SpikeReportReader::* )( ))
          &SpikeReportReader::getSpikes )
    .def( "getSpikes",
          ( Spikes (SpikeReportReader::* )( float, float ))
          &SpikeReportReader::getSpikes )
    .def( "hasEnded", &SpikeReportReader::hasEnded )
    .def( "isStream", &SpikeReportReader::isStream );

}
