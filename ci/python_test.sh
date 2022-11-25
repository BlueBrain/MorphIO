#!/usr/bin/env bash

set -euxo pipefail

VENV=env

function create_venv()
{
    if [[ "$OSTYPE" != "msys" ]]; then
        rm -rf "$VENV"
        python3 -mvenv "$VENV"

        set +u  # ignore errors in virtualenv's activate
        source "$VENV/bin/activate"
        set -u
    fi
}

# in tree install
create_venv

BIN=$VENV/bin/

$BIN/pip -v install .

which $BIN/pip

$BIN/pip install --upgrade pip setuptools wheel
$BIN/pip install -r tests/requirement_tests.txt

CURRENT=$(pwd)
(cd .. && pytest ${CURRENT}/tests)
deactivate

# sdist install
DIST_DIR="$VENV/dist"
mkdir -p "$DIST_DIR"
create_venv
python3 setup.py sdist --dist-dir "$DIST_DIR"
$BIN/pip install "$DIST_DIR"/MorphIO*.tar.gz
$BIN/pip install -r tests/requirement_tests.txt

CURRENT=$(pwd)
(cd .. && pytest ${CURRENT}/tests)
