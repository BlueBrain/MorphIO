# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-filesystem-dev, libboost-regex-dev,
  libboost-serialization-dev, libboost-system-dev, libboost-test-dev,
  libhdf5-serial-dev, ${LUNCHBOX_DEB_DEV_DEPENDENCY},
  ${VMMLIB_DEB_DEV_DEPENDENCY}")

include(CommonCPack)
