#!/usr/bin/env bash

# This script is run in the 'before_install' step of .travis.yaml
if [[ $TRAVIS_OS_NAME = linux ]]; then
    sudo apt-get install libhdf5-dev
else
    brew list hdf5 || brew install hdf5
fi
