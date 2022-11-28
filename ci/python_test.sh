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

        $VENV/bin/pip install --upgrade pip setuptools wheel
    fi
}

# in tree install
create_venv

$VENV/bin/pip -v install --force .
$VENV/bin/pip -v install -r tests/requirement_tests.txt


pushd $(pwd)/tests
pytest .
popd

deactivate

# sdist install
DIST_DIR="$VENV/dist"
mkdir -p "$DIST_DIR"
create_venv

python3 -m pip install build
python3 -m build . --outdir "$DIST_DIR"
python3 -m pip install "$DIST_DIR"/MorphIO*.tar.gz
python3 -m pip install -r tests/requirement_tests.txt

pushd $(pwd)/tests
pytest .
popd
