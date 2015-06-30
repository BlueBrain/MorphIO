# Copyright (c) 2015, EPFL/Blue Brain Project
#                     Stefan.Eilemann@epfl.ch
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(SPIKECONVERTER_HEADERS)
set(SPIKECONVERTER_SOURCES spikeConverter.cpp)
set(SPIKECONVERTER_LINK_LIBRARIES Brion)

common_application(spikeConverter)

set(MORPHOLOGYCONVERTER_HEADERS)
set(MORPHOLOGYCONVERTER_SOURCES morphologyConverter.cpp)
set(MORPHOLOGYCONVERTER_LINK_LIBRARIES Brion ${Boost_PROGRAM_OPTIONS_LIBRARY})

common_application(morphologyConverter)
