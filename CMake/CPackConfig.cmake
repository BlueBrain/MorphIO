# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")

set(BRION_PACKAGE_DEB_DEPENDS libboost-filesystem-dev libboost-regex-dev
  libboost-serialization-dev libboost-system-dev libboost-test-dev
  libhdf5-serial-dev)
if(USE_PYTHON3)
  set(PYTHON_SUFFIX 3)
endif()

if(TARGET brain_python)
  list(APPEND CPACK_DEBIAN_PACKAGE_DEPENDS libboost-python-dev
    python${PYTHON_SUFFIX}-numpy)
endif()

include(CommonCPack)
