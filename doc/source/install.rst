Install
=======

Dependencies
------------

To build MorphIO from sources, the following dependencies are required:

* cmake >= 3.2
* libhdf5-dev
* A C++11 compiler

Debian:

.. code-block:: shell

   sudo apt install cmake libhdf5-dev

Red Hat:

.. code-block:: shell

   sudo yum install cmake3.x86_64 hdf5-devel.x86_64

Max OS:

.. code-block:: shell

   brew install hdf5 cmake


Install as a C++ library
------------------------

For manual installation:

.. code-block:: shell

   git clone git@github.com:bluebrain/morphio.git --recursive
   cd morphio
   mkdir build && cd build
   cmake ..
   make install

To use the installed library:

.. code-block:: CMake

   find_package(MorphIO REQUIRED)
   target_link_libraries(mylib MorphIO::morphio)


Install as a Python package
---------------------------

The python binding can directly be installed using pip:

.. code-block:: shell

   pip install morphio
