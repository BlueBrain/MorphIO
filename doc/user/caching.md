Caching support
============

Brion and Brain have optional caching support for speeding up loading of various
data. The caching is based on the lunchbox::PersistentMap API which has several
key-value store backends which can be activated by the following environment
variables:

* MEMCACHED_SERVERS: a comma-separated list of servers with optional :port to
                     use memcached as a cache
* LEVELDB_CACHE: a path to the leveldb storage to use leveldb as a cache

## Cached data support

* Morphologies from brain::Circuit::loadMorphologies()
  * each morphology is hashed by its canonical filepath if COORDINATES_LOCAL
  * each morphology is hashed by its canonical filepath plus canonical circuit
    filepath and GID if COORDINATES_GLOBAL
* Synapse position from brain::Circuit::get<type>Synapses()
  * all synapse positions per neuron are hashed by its canonical filepath of the
    nrn file plus if afferent/efferent plus the GID of the neuron.
