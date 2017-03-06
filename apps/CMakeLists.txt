# Copyright (c) 2015-2017, EPFL/Blue Brain Project
#                          Stefan.Eilemann@epfl.ch
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(SPIKECONVERTER_HEADERS)
set(SPIKECONVERTER_SOURCES spikeConverter.cpp)
set(SPIKECONVERTER_LINK_LIBRARIES Brion ${Boost_PROGRAM_OPTIONS_LIBRARY})


set(MORPHOLOGYCONVERTER_HEADERS)
set(MORPHOLOGYCONVERTER_SOURCES morphologyConverter.cpp)
set(MORPHOLOGYCONVERTER_LINK_LIBRARIES Brion ${Boost_PROGRAM_OPTIONS_LIBRARY})

set(COMPARTMENTCONVERTER_HEADERS)
set(COMPARTMENTCONVERTER_SOURCES compartmentConverter.cpp)
set(COMPARTMENTCONVERTER_LINK_LIBRARIES Brion ${Boost_PROGRAM_OPTIONS_LIBRARY})

if(TARGET BBPTestData)
  list(APPEND COMPARTMENTCONVERTER_LINK_LIBRARIES BBPTestData)
  list(APPEND SPIKECONVERTER_LINK_LIBRARIES BBPTestData)
endif()

common_application(spikeConverter)
common_application(morphologyConverter)
common_application(compartmentConverter)
common_smoke_test(compartmentConverter -i dummy://?size=64 -o dummy://)
