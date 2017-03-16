.. _python_tutorial:

Brain Python tutorial
=====================

Brain is a high level API for read only access of the data model used in
the Blue Brain. This tutorial will describe the Python module API for
accessing the following data:

 * Circuit
 * Synapses
 * Morphologies
 * Spikes

These are the only parts of the data model that are accessible so far.
In particular, compartment reports and electrical models are not
available yet.

If you are reading the read-only version of this document and want to
access the actual ipynb file you can get it from here_.

.. _here: https://github.com/BlueBrain/Brion/blob/master/doc/python/python%20tutorial.ipynb

Before you start you have to make sure that you have the Python module
available in your environemnt. In the Blue Brain clusters you have two
possibilities depending on the Python version you want to use:

 * For Python 3.4: ``module load BBP/viz/latest``
 * For Python 2.7: ``module load memcache/bbpviz nix/viz/brion/2.0 nix/python2.7-full``

.. code:: python

    import brain
    import numpy

Circuits
--------

Before anything else, we need to have the metada for the circuit we want
to inspect. This can be easily done by creating a ``Circuit`` object
with the path to the CircuitConfig:

.. code:: python

    config = "/gpfs/bbp.cscs.ch/project/proj1/circuits/SomatosensoryCxS1-v5.r0/O1/merged_circuit/CircuitConfig"
    # If the circuit above is not accessible this is another alternative:
    #config = "/gpfs/bbp.cscs.ch/release/l2/2012.07.23/circuit/O1/merged_circuit/CircuitConfig"
    
    circuit = brain.Circuit(config)

The most basic information we can obtain from a circuit is: \* GID sets
from target names \* Neuron positions \* Morphological and
electrophysiological (ME) types. This information can be read for either
mvd2 or mvd3 circuit files.

.. code:: python

    # This returns all the GIDs of the circuit
    gids = circuit.gids()
    # But for demonstration purposes a minicolumn is just enough.
    gids = circuit.gids('MiniColumn_0')
    # The function gids returns a numpy array with the GIDs sorted in increasing order
    print(gids)

.. parsed-literal::

    [  341   802   903  1447  1543  1962  1990  2126  2224  2335  2567  2624
      2662  3165  3232  3440  4108  4374  4514  4628  5675  5794  6711  7302
      7664  8122  8164  8482  8624  9068  9640  9704 10195 10344 10499 10880
     11112 11423 11511 11601 11648 11957 12090 12134 12750 13119 13706 14245
     14422 14558 14746 14807 15020 16018 16189 16570 16828 17634 18345 18553
     18760 18883 19016 19273 19648 19995 20121 21090 21362 21494 21628 21742
     21872 22234 22368 22878 23393 23893 24026 24162 24291 24406 25164 25535
     26160 26408 26532 26663 27658 27909 28029 28164 28419 28662 29414 29544
     29931 30461 30587 30957 31077 31331]


.. code:: python

    # The positions are returned as a 3 x n numpy array.
    positions = circuit.positions(gids)
    print(positions.shape)
    print(positions[0])

.. parsed-literal::

    (102, 3)
    [  367.86856079  1815.69592285   194.61106873]


.. code:: python

    # The methods to get the ME types of a cell set return index arrays.
    mtypes = circuit.morphology_types(gids)
    # The indices are used to get the actual names from a separate array.
    names = circuit.morphology_type_names()
    print(names[mtypes[0]])
    
    etypes = circuit.electrophysiology_types(gids)
    names = circuit.electrophysiology_type_names() # typenames
    print(names[etypes[0]])

.. parsed-literal::

    L23_PC
    cADpyr


.. code:: python

    # From the circuit it's also possible to get the 4x4 matrix transform and orientation
    # quaternion for each cell, these are again returned as numpy arrays.
    transforms = circuit.transforms(gids)
    rotations = circuit.rotations(gids)
    print(rotations[0:10])

.. parsed-literal::

    [[-0.         -0.8779915  -0.          0.47867629]
     [-0.         -0.95352584 -0.          0.30131117]
     [ 0.          0.18559326  0.          0.98262668]
     [-0.         -0.57609433 -0.          0.81738323]
     [ 0.          0.98932451  0.          0.14572905]
     [ 0.          0.49220368  0.          0.87048006]
     [-0.         -0.48694596 -0.          0.8734321 ]
     [ 0.          0.05243697  0.          0.99862427]
     [ 0.          0.37507021  0.          0.92699641]
     [-0.         -0.14789115 -0.          0.98900366]]


.. code:: python

    # All functions that take gids as an input parameter accept any Python iterable or numpy arrays of int32, uint32 or int64
    print(circuit.morphology_types({1, 10, 100}))
    print(circuit.morphology_types([1, 10, 100]))
    print(circuit.morphology_types([10, 1, 100]))
    print(circuit.morphology_types(numpy.array([100, 10, 1])))

.. parsed-literal::

    [4 3 5]
    [4 3 5]
    [3 4 5]
    [5 3 4]


.. code:: python

    # Invalid GID lists will throw ValueError or RuntimeError
    circuit.positions([1, 1, 2])

::


    ---------------------------------------------------------------------------

    ValueError                                Traceback (most recent call last)

    <ipython-input-8-6c40e1ced9bb> in <module>()
          1 # Invalid GID lists will throw ValueError or RuntimeError
    ----> 2 circuit.positions([1, 1, 2])
    

    ValueError: Repeated GID found


.. code:: python

    circuit.positions([0])

::


    ---------------------------------------------------------------------------

    RuntimeError                              Traceback (most recent call last)

    <ipython-input-9-9863689e7678> in <module>()
    ----> 1 circuit.positions([0])
    

    RuntimeError: Cell GID out of range: 0


.. code:: python

    circuit.positions([1000000])

::


    ---------------------------------------------------------------------------

    RuntimeError                              Traceback (most recent call last)

    <ipython-input-10-749cb12fdeb5> in <module>()
    ----> 1 circuit.positions([1000000])
    

    RuntimeError: Cell GID out of range: 1000000


Synapses
--------

From the circuit object we can query the afferent, efferent of a cell
set and projected synapses between two sets.

In Lugano there is an in-memory caching system configured, one of the
advantages of using Brain is that it uses this caching to speed up the
access to synapse datasets. Another advantage is that by default the
data is loaded on demand, which is convenient when you are interested
only in part of the synpase attributes.

.. code:: python

    # Loading only the connectivity information for all afferent synapses of mc2_Layer4
    gids = circuit.gids("mc0_Layer4")
    %time synapses = circuit.afferent_synapses(gids)
    # The second time it will be faster due to caching
    %time synapses = circuit.afferent_synapses(gids)
    print(len(synapses))

.. parsed-literal::

    CPU times: user 1.51 s, sys: 800 ms, total: 2.31 s
    Wall time: 58.9 s
    CPU times: user 444 ms, sys: 108 ms, total: 552 ms
    Wall time: 552 ms
    6031664


.. code:: python

    # All synapses attributes are accessible as independent numpy arrays.
    # After querying the synapses, the connectivity attributes are already in memory.
    post = synapses.post_gids()
    pre = synapses.pre_gids()
.. code:: python

    # add local coordinates relative to pre and post
    
    # The rest will be loaded on demand
    %time x = synapses.post_center_x_positions()
    # The position and model attributes are cached separately.
    # Once we access one of the coordinates, the rest are recovered much faster
    %time y = synapses.post_center_y_positions()

.. parsed-literal::

    CPU times: user 981 ms, sys: 1.1 s, total: 2.08 s
    Wall time: 8.44 s
    CPU times: user 0 ns, sys: 1e+03 µs, total: 1e+03 µs
    Wall time: 95.4 µs


Apart from the array access, synapse containers can also be iterated and
the attributes accessed using an object interface.

.. code:: python

    subset = [s for s in synapses] # Note that slicing operations, such as synapses[0:100] are not implemented
    # Accessing the model attributes will trigger the loading for all synapses in the container.
    %time c = subset[0].conductance()
    # And now all data is in memory
    %time d = subset[0].delay()

.. parsed-literal::

    CPU times: user 3.78 s, sys: 876 ms, total: 4.66 s
    Wall time: 1min 30s
    CPU times: user 0 ns, sys: 0 ns, total: 0 ns
    Wall time: 17.2 µs


This final snippet show how to query the synapses projected from one
cell set to another:

.. code:: python

    %time projections = circuit.projected_synapses(gids, circuit.gids("mc0_Layer3"))

.. parsed-literal::

    CPU times: user 1.21 s, sys: 492 ms, total: 1.7 s
    Wall time: 34 s


Something to consider is that Synapses containers are independent, that
is, they cannot share data between them, so if you load the same neurons
in different targets, the data will be duplicated in memory. On the
other hand, the cache servers are accessible for anybody.

Morphologies
------------

Morphological data can be accessed by directly opening morphology files
are loading them through the circuit. When loading from the circuit,
global or local coordinates can be choosen (contrary to BBPSDK, there's
no implicit handling of global vs local coordinates, but the user
explicitly states which ones are desired).

.. code:: python

    # Let's load a morphology directly from a file.
    # Instead of providing a hardcoded file path we will get it from the circuit
    uri = circuit.morphology_uris({1000})[0]
    morphology = brain.neuron.Morphology(uri)
    
    # Loading from the circuit is done using GIDs.
    # When morphologies are shared and local coordinates are requested the underlaying objects are also shared in memory.
    morphologies = circuit.load_morphologies({1000}, circuit.Coordinates.local)

A neuron morphology is mainly consists of a set of sections and a soma.
The Section object provides methods to traverse the section tree and to
query information about the sample points.

Some functions also provide access to the low raw data arrays.

.. code:: python

    # The soma object provides the centroid, radius and profile points if any.
    soma = morphology.soma()
    print(soma.centroid())
    print(soma.mean_radius())
    print(soma.profile_points())

.. parsed-literal::

    (1.907348590179936e-08, 1.5258788721439487e-07, 1.0728836485895954e-08)
    7.4684977531433105
    [[-1.45480001  9.6239996  -0.2296      0.        ]
     [-2.3448      7.48400021 -0.2296      0.        ]
     [-2.97480011  5.40399981 -0.1696      0.        ]
     [-4.21479988  3.79399991 -0.1696      0.        ]
     [-5.22480011  2.14400005 -0.1696      0.        ]
     [-6.06479979  0.79400003 -0.1696      0.        ]
     [-7.45480013 -1.00600004 -0.1696      0.        ]
     [-7.73479986 -2.46600008 -0.1696      0.        ]
     [-7.10480022 -3.39599991 -0.1696      0.        ]
     [-4.85480022 -4.296      -0.1696      0.        ]
     [-1.73479998 -6.05600023 -0.1696      0.        ]
     [ 0.69520003 -7.22599983 -0.0296      0.        ]
     [ 2.07520008 -8.42599964  0.0304      0.        ]
     [ 2.9052     -8.93599987  0.1104      0.        ]
     [ 3.89520001 -7.38600016  0.17039999  0.        ]
     [ 6.53520012 -6.27600002  0.17039999  0.        ]
     [ 8.06519985 -5.53599977  0.17039999  0.        ]
     [ 7.52519989 -3.57599998  0.17039999  0.        ]
     [ 6.13520002 -0.64600003  0.17039999  0.        ]
     [ 5.04519987  1.53400004  0.17039999  0.        ]
     [ 3.2651999   3.61400008  0.17039999  0.        ]
     [ 2.60520005  5.01399994  0.17039999  0.        ]
     [ 2.04520011  6.86399984  0.17039999  0.        ]
     [ 1.69519997  9.24400043  0.17039999  0.        ]
     [-1.32480001  9.71399975  0.17039999  0.        ]]


.. code:: python

    # Sections can be retrieved by ID or type.
    # By type
    axon = morphology.sections({brain.neuron.SectionType.axon})
    # By id. The conversion is needed because id takes an integer, but not a numpy.uint32
    axon = [morphology.section(int(id)) for id in morphology.section_ids({brain.neuron.SectionType.axon})]
.. code:: python

    # The soma cannot be converted to a section
    morphology.section(0) # throws

::


    ---------------------------------------------------------------------------

    RuntimeError                              Traceback (most recent call last)

    <ipython-input-19-d5d9f3cc3afc> in <module>()
          1 # The soma cannot be converted to a section
    ----> 2 morphology.section(0) # throws
    

    RuntimeError: The soma cannot be accessed as a Section


.. code:: python

    section = axon[0]
    # The methods to traverse the morphology graph are parent and children.
    # Both may return None for first order and terminal sections respectively
    print("Children:", [s.id() for s in section.children()])
    print("Parent:", section.parent())
    
    # To retrieve the sample locations and diameters the method is samples.
    print(section.samples())
    
    # This method can take a list of relative position to obtain linearly interpolated sample positions.
    print(section.samples([0, 0.5, 1.0]))
    
    # add convenience function for section, segment and position along segment
        
    # Distances to soma can be queries in a similar way
    d = section.sample_distances_to_soma()
    print(section.length(), d[-1] - d[0])

.. parsed-literal::

    Children: [2, 59]
    Parent: None
    [[  1.16173995e+00  -2.73743010e+00   6.85094023e+00   1.36000001e+00]
     [  3.54643011e+00  -8.35649014e+00   2.09137001e+01   1.36000001e+00]
     [  1.18355000e+00  -1.71742992e+01   2.14995995e+01   6.00000024e-01]
     [  3.42235155e-02  -2.16313000e+01   2.24328995e+01   6.00000024e-01]
     [ -8.28444004e-01  -2.48330994e+01   2.32877007e+01   6.00000024e-01]
     [ -2.00281000e+00  -2.91338997e+01   2.45149002e+01   6.00000024e-01]
     [ -2.91564989e+00  -3.27745018e+01   2.55701008e+01   6.00000024e-01]
     [ -4.48455000e+00  -3.99515991e+01   2.81375999e+01   6.00000024e-01]
     [ -5.68577003e+00  -4.59066010e+01   3.07444992e+01   6.00000024e-01]
     [ -6.19869995e+00  -4.87081985e+01   3.19990997e+01   6.00000024e-01]
     [ -6.71611977e+00  -5.23386002e+01   3.35430984e+01   6.00000024e-01]
     [ -6.99650002e+00  -5.51487007e+01   3.44781990e+01   6.00000024e-01]
     [ -7.28359985e+00  -6.08227005e+01   3.58979988e+01   6.00000024e-01]
     [ -7.14223003e+00  -6.74588013e+01   3.67365990e+01   6.00000024e-01]
     [ -6.79067993e+00  -7.09904022e+01   3.70312004e+01   6.00000024e-01]
     [ -6.44088984e+00  -7.37704010e+01   3.71702003e+01   6.00000024e-01]
     [ -5.58654022e+00  -8.05896988e+01   3.69855003e+01   6.00000024e-01]
     [ -5.24412012e+00  -8.43290024e+01   3.66076012e+01   6.00000024e-01]
     [ -4.36117983e+00  -9.24504013e+01   3.53945999e+01   6.00000024e-01]
     [ -3.78156996e+00  -9.72034988e+01   3.43694992e+01   6.00000024e-01]
     [ -3.46371007e+00  -1.00455002e+02   3.35546989e+01   6.00000024e-01]
     [ -3.10776997e+00  -1.07380997e+02   3.18061008e+01   6.00000024e-01]
     [ -3.05665994e+00  -1.14172997e+02   3.01411991e+01   6.00000024e-01]]
    [[   1.16173995   -2.7374301     6.85094023    1.36000001]
     [  -6.74434948  -52.62153244   33.63724899    0.60000002]
     [  -3.05665994 -114.17299652   30.14119911    0.60000002]]
    125.8109130859375 125.811


Spikes
------

Spikes can be read from the usual ``out.dat`` ASCII file, ``.gdf`` files
generated by NEST and a custom ``.spikes`` binary file format that can
be generated with the spikeConverter tool. For any of them the code is
the same, first create the reader with the filepath and then request the
synapses inside a time window:

.. code:: python

    # Loading small simulations from ASCII doesn't take a lot of time.
    %time reader = brain.SpikeReportReader('/gpfs/bbp.cscs.ch/project/proj3/simulations/vizCa2p0_1x7/out.dat')

.. parsed-literal::

    CPU times: user 2.06 s, sys: 62 ms, total: 2.12 s
    Wall time: 913 ms


.. code:: python

    # For bigger ones it's better to convert the data to binary first. 
    # This is a 74 million cell report from a NEST simulation that has been converted to binary.
    # The conversion step took around one hour and the resulting file is 2.6 GB.
    %time reader = brain.SpikeReportReader('/gpfs/bbp.cscs.ch/project/proj3/resources/simulations/till_viztm659/till.spikes')

.. parsed-literal::

    CPU times: user 19.2 s, sys: 5.96 s, total: 25.2 s
    Wall time: 25.1 s


.. code:: python

    # The actual time interval of the report cannot be known by the reader. Instead, it provides the 
    # time of the first and last spike.
    print (reader.getStartTime(), reader.getEndTime())

.. parsed-literal::

    3.700000047683716 79.80000305175781


.. code:: python

    # Spikes are queried giving a time window. The object returned can be iterated.
    %time spikes = reader.getSpikes(10, 30)
    print(len(spikes))

.. parsed-literal::

    CPU times: user 2.81 s, sys: 8 ms, total: 2.82 s
    Wall time: 2.8 s
    75180145


One final remark regarding API stability, there are plans for a new API
at the C++ side for better efficiency loading very large spike reports
and it's not clear if this will imply changes in the Python API.
