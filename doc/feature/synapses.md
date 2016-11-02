Synapse support in Brain {#brainSynapses}
============

The low-level data access for synapses provided by Brion needs a complementary
API in Brain to support various common higher-level uses cases, like semantic
synapse object access or full-circuit synapse density voxelization, to name the
two extremes.

## Requirements

* Semantic synapse objects that give read-access to all properties
* Array access which allows vectorization
* Filtering of synapses for pathway use case (projected synapses of two targets)
* Lazy-loading to support out-of-core scenarios
* User-transparent KVS caching

## New dependencies

None

## API

```cpp

namespace brain
{
    /*
     * Synapse GID.
     *
     * The GID of a synapse is the a tuple of two numbers:
     * - The GID of the post-synaptic cell.
     * - The index of the synapse in the array of afferent contacts
     *   of the post-synaptic cell before pruning/filtering.
     * GIDs are invariant regardless of how the structural touches are
     * converted into functional synapses during circuit building.
     */
    typedef std::pair< uint32_t, size > SynapseGID;

    // loading of data during SynapseStream.read(), otherwise load happens on
    // first touch
    enum SynapsesPrefetch
    {
        SP_NONE = 0,            // only loads pre- and post GIDs
        SP_ATTRIBUTES = 1 << 0, // topological information (section, segment,
                                // distance) and electrical attributes
        SP_POSITIONS = 1 << 1,  // pre/post surface/center positions
        SP_ALL = SP_ATTRIBUTES | SP_POSITIONS
    }
}

class Circuit
{
    // max count for read is gids.size()
    SynapsesStream getAfferentSynapses( const GIDSet& gids,
                                     SynapsesPrefetch prefetch = SP_ALL ) const;

    // max count for read is gids.size()
    SynapsesStream getEfferentSynapses( const GIDSet& gids,
                                     SynapsesPrefetch prefetch = SP_ALL ) const;

    // max count for read is min(preGIDs.size(), postGIDs.size())
    SynapsesStream getProjectedSynapses( const GIDSet& preGIDs,
                                         const GIDSet& postGIDs,
                                     SynapsesPrefetch prefetch = SP_ALL ) const;
};

// read-only, moveable proxy object of data storage in Synapses container
class Synapse
{
    uint32_t preGID() const;
    uint32_t preSectionID() const;
    uint32_t preSegmentID() const;
    float preDistance() const;
    Vector3f preSurfacePosition() const;
    Vector3f preCenterPosition() const;

    uint32_t postGID() const;
    uint32_t postSectionID() const;
    uint32_t postSegmentID() const;
    float postDistance() const;
    Vector3f postSurfacePosition() const;
    Vector3f postCenterPosition() const;

    SynapseGID gid() const;
    float conductance() const;
    float utilization() const;
    float depression() const;
    float facilitation() const;
    float decay() const;
    int efficacy() const;
};

// read-only, moveable, copyable, thread-safe
class Synapses
{
    // conversion ctor; read all synapses from selected GIDs into memory
    Synapses( const SynapsesStream& );

    size_t size() const;
    bool empty() const;

    bool operator==( const Synapses& rhs ) const;
    bool operator!=( const Synapses& rhs ) const;

    const_iterator begin() const;
    const_iterator end() const;

    const Synapse& operator[]( size_t ) const;

    const size_t* index() const;
    const uint32_t* preGID() const;
    const uint32_t* preSectionID() const;
    const uint32_t* preSegmentID() const;
    const float* preDistance() const;
    const float* preSurfacePositionX() const;
    const float* preSurfacePositionY() const;
    const float* preSurfacePositionZ() const;
    const float* preCenterPositionX() const;
    const float* preCenterPositionY() const;
    const float* preCenterPositionZ() const;

    const uint32_t* postGID() const;
    const uint32_t* postSectionID() const;
    const uint32_t* postSegmentID() const;
    const float* postDistance() const;
    const float* postSurfacePositionX() const;
    const float* postSurfacePositionY() const;
    const float* postSurfacePositionZ() const;
    const float* postCenterPositionX() const;
    const float* postCenterPositionY() const;
    const float* postCenterPositionZ() const;

    const size_t* indices() const;
    const float* conductance() const;
    const float* utilization() const;
    const float* depression() const;
    const float* facilitation() const;
    const float* decay() const;
    const int* efficacy() const;
};

// sequentially & forwarding iteration through synapses for any number of GIDs
// not thread-safe, but reentrant
class SynapsesStream
{
    bool eos() const;

    // Remaining count reads before eos()
    // equals to max - current, max determined by get..Synapses() in
    // brain::Circuit
    size_t getRemaining() const;

    // - async load of synapses for count cells
    // - no order guarantee
    // - resulting Synapses can be empty
    // - count will be clamped if count > getRemaining()
    std::future< Synapses > read( size_t count = 1 ) const;
};

```

## File format

Uses existing nrn* files

## Examples

### Stream synapse positions for pathway density voxelization

```cpp
const brain::Circuit circuit;
const brain::SynapsesStream& stream =
   circuit.getProjectedSynapses( circuit.getGIDs( "mtypetarget_1" ),
                                 circuit.getGIDs( "mtypetarget_2" ));
std::future< brain::Synapses > future = stream.read();
while( !stream.eos( ))
{
    const brain::Synapses synapses = future.get();
    future = stream.read(); // fetch next

    const float* __restrict__ posx = synapses.preSurfacePositionX();
    const float* __restrict__ posy = synapses.preSurfacePositionY();
    const float* __restrict__ posz = synapses.preSurfacePositionZ();
    for( size_t i = 0; i < synapses.size(); ++i )
        doVoxelization( posx[i], posy[i], posz[i] );
}
```

### Access all afferent synapses of a GIDSet

```cpp
const brain::Circuit circuit;
const brain::Synapses& synapses =
    circuit.getAfferentSynapses( circuit.getGIDs( "Layer1" ));
for( const auto& synapse : synapses )
    std::cout << synapse.postGID() << " " << synapse.decay() << std::endl;
```

### Retrograde projection

```cpp
const brain::Circuit circuit;
const brion::GIDSet& preNeurons = circuit.getGIDs( "Layer1" );
const brion::GIDSet postNeuron = { 1 };
const brain::Synapses& synapses =
    circuit.getProjectedSynapses( preNeurons, postNeuron );
BOOST_CHECK( !synapses.empty( ));
BOOST_CHECK_EQUAL( synapses.size(), 5 );
```

## Implementation

* Data owned by container in the form of the arrays (transformed from
  SynapseMatrix), loaded upon construction wrt prefetch, lazy on first touch
  except for pre- and postGID for connectivity queries and to fulfill the
  contract for size().
* Synapse object stores index to get data from (parenting) container, triggers
  lazy loading on first touch.
* SynapsesStream::read() uses async read of synapse data of the given fraction
  of GIDs.
* OPT: getProjectedSynapses() loads the synapses from the smaller GIDSet and
  uses the correct side (afferent/efferent) accordingly.

## Issues

### 1: Why does the synapses container has to load on construction?

_Resolved: Yes_

For an easier implemenation and a consistent API. Determining the size of the
container already requires 'touching' all synapse datasets in the current file
storage, hence loading attributes or positions in the same go does not waste
more time. Regarding memory concerns, the stream approach helps there to only
load synapses for a fraction of gids of interest.

### 2: Why there are no set operators like in the BBPSDK synapse container?

_Resolved: Yes_

Synapses::getProjectedSynapses() was the missing part in the SDK to render most
of the set operations useless. It is a more optimized way of filtering.
