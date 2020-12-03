#!/usr/bin/env bash

set -euxo pipefail

VENV=$(pwd)/build/venv-python-test/

if [[ ! -d "$VENV" ]]; then
    python3 -mvenv "$VENV"
fi

BIN=$VENV/bin/

source $BIN/activate
pip -v install --upgrade pip setuptools wheel

# install
pip -v install --force .
pip install nose h5py

nosetests -s -v -P tests
