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

#ifndef BRAIN_PYTHON_TYPES
#define BRAIN_PYTHON_TYPES

#include <brain/synapse.h>
#include <brain/synapses.h>
#include <brain/types.h>

namespace brain
{
typedef boost::shared_ptr<Circuit> CircuitPtr;

/** Synapse container wrapping helper.

   This class is used to keep a valid reference to the circuit together with any
   Synapse container held by a Python variable.
*/
class SynapsesWrapper : public Synapses
{
public:
    SynapsesWrapper(const SynapsesStream& stream, const CircuitPtr& circuit)
        : Synapses(stream)
        , _circuit(circuit)
    {
    }

    CircuitPtr _circuit;

    // Unprotecting the _impl member because it needs to be accessed by the
    // wrapping to use it as custodian.
    using Synapses::_impl;
};

/** Synapse container wrapping helper.

   This class is used to keep a valid reference to the circuit together with any
   Synapse container held by a Python variable.
*/
class SynapseWrapper : public Synapse
{
public:
    SynapseWrapper(Synapse&& synapse, const Synapses& synapses,
                   const CircuitPtr& circuit)
        : Synapse(std::move(synapse))
        , _synapses(synapses)
        , _circuit(circuit)
    {
    }

    Synapses _synapses;
    CircuitPtr _circuit;
};
}
#endif
