#!/usr/bin/env bash

set -euxo pipefail

VENV=env

rm -rf "$VENV"
python3 -mvenv "$VENV"

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

pip install --upgrade pip setuptools

# install
pip install .

pip install -r tests/requirement_tests.txt
nosetests
