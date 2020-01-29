#!/usr/bin/env bash

set -euxo pipefail

VENV=env

rm -rf "$VENV"
virtualenv -p $(which python) "$VENV"

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

pip install --upgrade pip setuptools

# install
pip install .

pip install -r tests/requirement_tests.txt
nosetests
