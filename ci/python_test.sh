#!/usr/bin/env bash

set -euxo pipefail

VENV=env

if [[ "$OSTYPE" != "msys" ]]; then
    rm -rf "$VENV"
    python3 -mvenv "$VENV"

    set +u  # ignore errors in virtualenv's activate
    source "$VENV/bin/activate"
    set -u

    which pip

    pip install --upgrade pip setuptools
fi

pip install .
pip install -r tests/requirement_tests.txt

CURRENT=$(pwd)
cd ..
pytest ${CURRENT}/tests
