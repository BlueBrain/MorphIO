#!/usr/bin/env bash

set -euxo pipefail

VENV=env

rm -rf "$VENV"
python3 -mvenv "$VENV"

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

which pip

pip install --upgrade pip setuptools

# install
pip install -vvv .

pip install -r tests/requirement_tests.txt
CURRENT=$(pwd)

cd ..
nosetests ${CURRENT}/tests
