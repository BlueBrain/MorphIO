import sys as _sys

if _sys.version_info[0] != ${USE_PYTHON_VERSION}:
    raise ImportError("Invalid Python version")

from ._brain import *
