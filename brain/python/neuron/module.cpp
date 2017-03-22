/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "../submodules.h"

#include <brain/neuron/types.h>

namespace brain
{
namespace neuron
{
void export_Morphology();

// clang-format off
void export_module()
{
    boost::python::scope neuron = exportSubmodule("neuron");

    boost::python::enum_< neuron::SectionType >( "SectionType" )
        .value( "soma", neuron::SectionType::soma )
        .value( "axon", neuron::SectionType::axon )
        .value( "dendrite", neuron::SectionType::dendrite )
        .value( "apical_dendrite", neuron::SectionType::apicalDendrite )
        .value( "undefined", neuron::SectionType::undefined );

    export_Morphology();
}
// clang-format on
}
}
