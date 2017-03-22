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

#include "submodules.h"

namespace brain
{
namespace bp = boost::python;

bp::scope exportSubmodule(const std::string& name)
{
    // I'm not fully sure that the module stack is left in a conventional
    // state after this code is run. For example the module neuron appears
    // as built-in in the interpreter, which smells fishy.
    // Nonetheless, the current implementation allows to do:
    //
    // import _brain
    // import _brain._neuron
    // from _brain._neuron import X (auto-completion of X works in IPython)
    //
    // And that's good enough.

    bp::object module(bp::handle<>(
        bp::borrowed(PyImport_AddModule(("brain._brain._" + name).c_str()))));
    bp::scope().attr(("_" + name).c_str()) = module;
    bp::scope moduleScope = module;
    // Despite these paths are not completely true in the build directory,
    // they ensures that _brain._neuron can be found and that
    // _brain.so is not loaded twice.
    moduleScope.attr("__package__") = "brain._brain";
    moduleScope.attr("__path__") = "brain._brain";

    return moduleScope;
}
}
