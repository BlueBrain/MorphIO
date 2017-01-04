/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
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

#include "compartmentReportCommon.h"
#include <lunchbox/log.h>

namespace brion
{
namespace plugin
{

CompartmentReportCommon::CompartmentReportCommon()
{

}

void CompartmentReportCommon::_cacheNeuronCompartmentCounts(
                                                    const GIDSet& gids )
{
    if( gids.empty() || gids != getGIDs() || _neuronCompartments.empty( ))
    {
        updateMapping( gids );
        const CompartmentCounts& counts = getCompartmentCounts();
        _neuronCompartments.resize( counts.size( ) );
        for( size_t i = 0; i < counts.size(); ++i )
            _neuronCompartments[i] = std::accumulate( counts[i].begin(),
                                                      counts[i].end(), 0 );
    }
}

size_t CompartmentReportCommon::getNumCompartments( const size_t index ) const
{
    return _neuronCompartments[ index ];
}

size_t CompartmentReportCommon::_getFrameNumber( float timestamp ) const
{
    timestamp = std::max( 0.f, timestamp - getStartTime( ));
    size_t frame =
        std::min(size_t(round((getEndTime() - getStartTime( )) /
                               getTimestep( ))) - 1,
                 size_t(round(timestamp / getTimestep( ))));
    return frame;
}

GIDSet CompartmentReportCommon::_computeIntersection( const GIDSet& all,
                                                      const GIDSet& subset )
{
    GIDSet intersection;
    std::set_intersection( subset.begin(), subset.end(), all.begin(), all.end(),
                           std::inserter( intersection, intersection.begin( )));
    if( intersection != subset || intersection.empty( ))
    {
        LBWARN << "Requested " << subset.size() << " GIDs [" << *subset.begin()
               << ":" <<*subset.rbegin() << "] are not a subset of the "
               << all.size() << " GIDs in the report [" << *all.begin() << ":"
               << *all.rbegin();
        if( intersection.empty( ))
            LBWARN << " with no GIDs in common" << std::endl;
        else
            LBWARN << "], using intersection size " << intersection.size()
                   << " [" << *intersection.begin() << ":"
                   << *intersection.rbegin() << "]" << std::endl;
    }
    return intersection;
}

}
}
