Python bindings for Morpho tool.
-------------------------------

These Python bindings require morpho-tool as a shared lib. 
Therefore be sure morphotool library is already in the system before installing this package with pip

Otherwise, building morpho-tool with cmake will build the bindings together and, make install will install the python package to a location pointed by prefix.
Examples:

1. Build only, no writing outside project folder
    mkdir build && cd build && cmake ..

2. Install everything to a given directory
    Build (step 1.)
    make install -DCMAKE_INSTALL_PREFIX=<directory>
NOTE: You might need to set you PYTHONPATH

3. Install Python lib to current Python (system-wide or activated virtualenv)
    Build (step 1.)
    make install -DCMAKE_INSTALL_PREFIX=<directory> -DPY_INSTALL_OPTIONS=

