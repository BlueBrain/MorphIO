#!/usr/bin/env bash

set -euxo pipefail

VENV=env

pip install virtualenv
rm -rf "$VENV"
virtualenv "$VENV"

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

pip install --upgrade pip

# install
pip install .

pip install -r tests/requirement_tests.txt
nosetests
