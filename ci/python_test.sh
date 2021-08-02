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

        which pip

        pip install --upgrade pip setuptools wheel
    fi
}

# in tree install
create_venv
# https://github.com/pypa/pip/issues/7555
pip install --use-feature=in-tree-build .
pip install -r tests/requirement_tests.txt

CURRENT=$(pwd)
(cd .. && pytest ${CURRENT}/tests)
deactivate

# sdist install
DIST_DIR="$VENV/dist"
mkdir -p "$DIST_DIR"
create_venv
python3 setup.py sdist --dist-dir "$DIST_DIR"
pip install "$DIST_DIR"/MorphIO*.tar.gz
pip install -r tests/requirement_tests.txt

CURRENT=$(pwd)
(cd .. && pytest ${CURRENT}/tests)
