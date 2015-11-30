# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                          Juan Hernando <jhernando@fi.upm.es>
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

source_group(\\ FILES CMakeLists.txt)

include(files.cmake)

set(BRAIN_PUBLIC_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
set(BRAIN_LINK_LIBRARIES PUBLIC Brion vmmlib)
set(BRAIN_INCLUDE_NAME brain)
set(BRAIN_NAMESPACE brain)

common_library(Brain)
