#!/usr/bin/env bash

# Install the desired clang-format, check the lines
# changed by this diff are formatted correctly

set -euo pipefail

VENV=venv-clang-format
CLANG_FORMAT_VERSION=9.0.0

if [[ ! -d $VENV ]]; then
    python3 -mvenv "$VENV"
    "$VENV/bin/pip" install --upgrade pip
    "$VENV/bin/pip" install clang-format=="$CLANG_FORMAT_VERSION"
fi

set +u  # ignore errors in virtualenv's activate
source "$VENV/bin/activate"
set -u

changes=$(git-clang-format 'HEAD~1')
if [[ $(echo "$changes" | grep -n1 'changed files') ]]; then
    echo "The following files require changes to pass the current clang-format"
    echo "$changes"
    echo "Run ./ci/check_clang_format.sh to apply the required changes to your repo"
    exit 1
fi
