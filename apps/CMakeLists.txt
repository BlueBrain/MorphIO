# Copyright (c) 2015, EPFL/Blue Brain Project
#                     Stefan.Eilemann@epfl.ch
#
# This file is part of Brion <https://github.com/BlueBrain/Brion>
#

set(SPIKECONVERTER_HEADERS)
set(SPIKECONVERTER_SOURCES spikeConverter.cpp)
set(SPIKECONVERTER_LINK_LIBRARIES Brion)

common_application(spikeConverter)
