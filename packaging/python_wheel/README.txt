Overview
********

This directory contains files required to build python wheels
for the Brain bindings.

It assumes a docker setup, and will produce wheels for the python versions
as described in `docker_build_wheel.sh:PYTHON_VERSIONS`

It uses the manywheel1 docker image from `https://github.com/pypa/manylinux/`
but doesn't customize it further.  Instead, `build_wheel.sh` is executed from
outside the container, preparing the externally needed source distributions,
and getting the required dependencies for Brion/Brain.

This then launches the vanilla manylinux1 container, and runs
`docker_build_wheel.sh`, which compiles CMake, HDF5 and Boost. It then builds
the wheels for each of the required python versions.

To debug the build process, one can enter the container, and manually run
the `docker_build_wheel.sh`:

    $ BASE=$(git rev-parse --show-toplevel)
    $ docker run -it --rm \
        -v $BASE:/io quay.io/pypa/manylinux1_x86_64 \
        /bin/bash
    $ cd /io/packaging/python_wheel

Possible Improvements
*********************
 - A custom Dockerfile could be created so that CMake/HDF5 isn't rebuilt each
   time, however, the time saving from this wouldn't be large, and potentially
   complicates the deployment.
 - Only recompile BoostPython, instead of all of Boost
 - Revisit hardcoded versions of dependencies which are used by CMake and wheel
   build, e.g. numpy, and source dependencies like boost

Current limitations
*******************
  - No OpenMP support, thus compartment reports are slower
  - No memcached, thus no caching of morphologies and synapses
