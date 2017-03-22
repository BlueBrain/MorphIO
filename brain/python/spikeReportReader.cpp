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

#include "arrayHelpers.h"
#include "docstrings.h"
#include "helpers.h"

#include <brain/spikeReportReader.h>
#include <brain/types.h>

namespace bp = boost::python;

namespace brain
{
namespace
{
SpikeReportReaderPtr _initURI(const std::string& uri)
{
    return SpikeReportReaderPtr(new SpikeReportReader(brion::URI(uri)));
}

SpikeReportReaderPtr _initURIandGIDSet(const std::string& uri, bp::object gids)
{
    return SpikeReportReaderPtr(
        new SpikeReportReader(brion::URI(uri), gidsFromPython(gids)));
}

bp::object SpikeReportReader_getSpikes(SpikeReportReader& reader,
                                       const float startTime,
                                       const float endTime)
{
    return toNumpy(reader.getSpikes(startTime, endTime));
}
}

void export_SpikeReportReader()
{
    const auto selfarg = bp::arg("self");

    // clang-format off
bp::class_< SpikeReportReader, boost::noncopyable >(
    "SpikeReportReader", bp::no_init )
    .def( "__init__", bp::make_constructor( _initURI ),
          DOXY_FN( brain::SpikeReportReader::SpikeReportReader( const brion::URI& )))
    .def( "__init__", bp::make_constructor( _initURIandGIDSet ),
          DOXY_FN( brain::SpikeReportReader::SpikeReportReader( const brion::URI&, const GIDSet& )))
    .def( "close", &SpikeReportReader::close,
          DOXY_FN( brain::SpikeReportReader::close ))
    .def( "get_spikes", SpikeReportReader_getSpikes,
          ( selfarg, bp::arg( "start_time" ), bp::arg( "stop_time" )),
          DOXY_FN( brain::SpikeReportReader::getSpikes ))
    .add_property( "end_time", &SpikeReportReader::getEndTime,
                   DOXY_FN( brain::SpikeReportReader::getEndTime ))
    .add_property( "has_ended", &SpikeReportReader::hasEnded,
                   DOXY_FN( brain::SpikeReportReader::hasEnded ))
    ;
    // clang-format on
}
}
