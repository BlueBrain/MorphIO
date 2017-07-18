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

#ifndef BRION_PLUGIN_COMPARTMENTREPORTCOMMON
#define BRION_PLUGIN_COMPARTMENTREPORTCOMMON

#include "../compartmentReportPlugin.h"
#include "../pluginInitData.h"
#include "../types.h"

namespace brion
{
namespace plugin
{
class CompartmentReportCommon : public CompartmentReportPlugin
{
public:
    CompartmentReportCommon();
    ~CompartmentReportCommon() {}
    size_t getNumCompartments(size_t index) const final;

    floatsPtr loadFrame(double timestamp) const final;
    Frames loadFrames(double start, double end) const final;
    size_t getFrameCount() const final;

protected:
    void _cacheNeuronCompartmentCounts(const GIDSet& gids);
    /** @return The frame number of a given timestamp clamped to the simulation
        window. */
    size_t _getFrameNumber(double timestamp) const;
    static GIDSet _computeIntersection(const GIDSet& all, const GIDSet& subset);

    virtual bool _loadFrame(size_t frameNumber, float* buffer) const = 0;
    virtual bool _loadFrames(size_t frameNumber, size_t frameCount,
                             float* buffer) const;

private:
    size_ts _neuronCompartments;
};
}
}

#endif
