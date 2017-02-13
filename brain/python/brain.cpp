/* Copyright (c) 2006-2016, Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#ifdef _MSC_VER
#pragma warning(disable : 4127)
#endif

#include <boost/python.hpp>

#include "arrayHelpers.h"

#include "docstrings.h"

#include "../compartmentReportReader.h"

#include <vmmlib/vector.hpp>

namespace brain
{
void export_Circuit();
void export_Spikes();
void export_SpikeReportReader();
void export_SpikeReportWriter();
void export_Synapses();
void export_test();
void export_CompartmentReportReader();

namespace neuron
{
void export_module();
}
}

struct Vector3fToTuple
{
    static PyObject* convert(const brain::Vector3f& v)
    {
        boost::python::object tuple =
            boost::python::make_tuple(v.x(), v.y(), v.z());
        return boost::python::incref(tuple.ptr());
    }
};

struct URItoString
{
    static PyObject* convert(const servus::URI& uri)
    {
        boost::python::object result(std::to_string(uri));
        return boost::python::incref(result.ptr());
    }
};


BOOST_PYTHON_MODULE(_brain)
{
#ifdef BRAIN_HAS_DOCSTRINGS
    /* Only change the default Boost.Python options for documentation if we
       are going to get docstrings from doxygen. */
    boost::python::docstring_options doc_options(true, true, false);
#endif

    boost::python::to_python_converter<servus::URI, URItoString>();
    boost::python::to_python_converter<brain::Vector3f, Vector3fToTuple>();

    brain::importArray();

    boost::python::enum_<brain::SynapsePrefetch>("SynapsePrefetch",
                                                 DOXY_ENUM(
                                                     brain::SynapsePrefetch))
        .value("none", brain::SynapsePrefetch::none)
        .value("attributes", brain::SynapsePrefetch::attributes)
        .value("positions", brain::SynapsePrefetch::positions)
        .value("all", brain::SynapsePrefetch::all);

    brain::neuron::export_module();

    brain::export_test();

    brain::export_Circuit();
    brain::export_Spikes();
    brain::export_SpikeReportReader();
    brain::export_SpikeReportWriter();
    brain::export_CompartmentReportReader();
    brain::export_Synapses();
}
