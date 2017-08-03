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

#ifndef BRION_TYPES
#define BRION_TYPES

#include <brion/enums.h>

#include <boost/multi_array.hpp>
#include <map>
#include <servus/uri.h>
#include <set>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <vmmlib/types.hpp>
#pragma warning(pop)

#ifdef __GNUC__
#define BRION_UNUSED __attribute__((unused))
#else
#define BRION_UNUSED
#endif

/** @namespace brion Blue Brain File IO classes */
namespace brion
{
using namespace enums;
class BlueConfig;
class Circuit;
class CompartmentReport;
class CompartmentReportPlugin;
class Mesh;
class Morphology;
class SpikeReport;
class SpikeReportPlugin;
class Synapse;
class SynapseSummary;
class Target;

using vmml::Vector2i;
using vmml::Vector3f;
using vmml::Vector4f;
using vmml::Vector3d;
using vmml::Vector4d;

typedef std::vector<size_t> size_ts;
typedef std::vector<int32_t> int32_ts;
typedef std::vector<uint16_t> uint16_ts;
typedef std::vector<uint32_t> uint32_ts;
typedef std::vector<uint64_t> uint64_ts;
typedef std::vector<float> floats;
typedef std::vector<double> doubles;
typedef std::vector<Vector2i> Vector2is;
typedef std::vector<Vector3f> Vector3fs;
typedef std::vector<Vector4f> Vector4fs;
typedef std::vector<Vector3d> Vector3ds;
typedef std::vector<Vector4d> Vector4ds;
typedef std::vector<SectionType> SectionTypes;
typedef std::vector<Target> Targets;
typedef std::shared_ptr<int32_ts> int32_tsPtr;
typedef std::shared_ptr<uint16_ts> uint16_tsPtr;
typedef std::shared_ptr<uint32_ts> uint32_tsPtr;
typedef std::shared_ptr<floats> floatsPtr;
typedef std::shared_ptr<doubles> doublesPtr;
typedef std::shared_ptr<Vector2is> Vector2isPtr;
typedef std::shared_ptr<Vector3fs> Vector3fsPtr;
typedef std::shared_ptr<Vector4fs> Vector4fsPtr;
typedef std::shared_ptr<Vector3ds> Vector3dsPtr;
typedef std::shared_ptr<Vector4ds> Vector4dsPtr;
typedef std::shared_ptr<SectionTypes> SectionTypesPtr;

/** Ordered set of GIDs of neurons. */
typedef std::set<uint32_t> GIDSet;

typedef GIDSet::const_iterator GIDSetCIter;
typedef GIDSet::iterator GIDSetIter;

/** The offset for the voltage per section for each neuron, uin64_t max for
 *  sections with no compartments.
 */
typedef std::vector<uint64_ts> SectionOffsets;

/** The number of compartments per section for each neuron. */
typedef std::vector<uint16_ts> CompartmentCounts;

/** Data matrix storing NeuronAttributes for each neuron. */
typedef boost::multi_array<std::string, 2> NeuronMatrix;

/** Data matrix storing SynapseAttributes for each neuron. */
typedef boost::multi_array<float, 2> SynapseMatrix;

/** Data matrix storing GID, numEfferent, numAfferent for each neuron. */
typedef boost::multi_array<uint32_t, 2> SynapseSummaryMatrix;

/** A spike */
typedef std::pair<float, uint32_t> Spike;
typedef std::vector<Spike> Spikes;

/** A list of Spikes events per cell gid, indexed by spikes times. */
typedef std::multimap<float, uint32_t> SpikeMap;

struct Frame
{
    double timestamp;
    floatsPtr data;
};

struct Frames
{
    doublesPtr timeStamps;
    /** The data of multiple compartment frames in a flat array. The number
        of frames equals timeStamp->size(). All frames have the same size,
        this size and the mapping of values to entities is defined in the
        report mapping. */
    floatsPtr data;
};

/** A value for undefined timestamps */

const float UNDEFINED_TIMESTAMP BRION_UNUSED =
    std::numeric_limits<float>::max();
const float RESTING_VOLTAGE BRION_UNUSED = -67.; //!< Resting voltage in mV
/** Lowest voltage after hyperpolarisation */
const float MINIMUM_VOLTAGE BRION_UNUSED = -80.;

typedef std::vector<std::string> Strings;

using servus::URI;
using URIs = std::vector<URI>;
}

// if you have a type T in namespace N, the operator << for T needs to be in
// namespace N too
namespace boost
{
template <typename T>
inline std::ostream& operator<<(std::ostream& os,
                                const boost::multi_array<T, 2>& data)
{
    for (size_t i = 0; i < data.shape()[0]; ++i)
    {
        for (size_t j = 0; j < data.shape()[1]; ++j)
            os << data[i][j] << " ";
        os << std::endl;
    }
    return os;
}
}

namespace std
{
template <class T, class U>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& pair)
{
    return os << "[ " << pair.first << ", " << pair.second << " ]";
}
}

#endif
