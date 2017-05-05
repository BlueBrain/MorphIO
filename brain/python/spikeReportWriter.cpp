/* Copyright (c) 2006-2017, Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include <brain/spikeReportWriter.h>
#include <brain/types.h>

#include "arrayHelpers.h"
#include "docstrings.h"

namespace bp = boost::python;

namespace brain
{
namespace
{
SpikeReportWriterPtr _initURI(const std::string& uri)
{
    return SpikeReportWriterPtr(new SpikeReportWriter(brion::URI(uri)));
}

void SpikeReportWriter_writeSpikes(SpikeReportWriter& writer,
                                   const bp::object& object)
{
    try
    {
        const auto spikes = spikesFromNumpy(object);
        writer.writeSpikes(spikes.first, spikes.second);
    }
    catch (bp::error_already_set&)
    {
        PyErr_Clear();
        // Try again extracting the spikes for a list of tuples
        Spikes spikes;
        try
        {
            bp::stl_input_iterator<bp::tuple> i(object), end;
            while (i != end)
            {
                bp::tuple t = *i++;
                if (bp::len(t) != 2)
                    boost::python::throw_error_already_set();
                const float time = bp::extract<double>(t[0]);
                const uint32_t gid = bp::extract<uint32_t>(t[1]);
                spikes.push_back(std::make_pair(time, gid));
            }
        }
        catch (bp::error_already_set&)
        {
            PyErr_SetString(
                PyExc_ValueError,
                "Cannot convert argument Spike array. Only "
                "numpy arrays of dtype=\"f4, u4\" and iterables of (float, int)"
                " tuples are accepted");
            throw;
        }
        writer.writeSpikes(spikes);
    }
}
}

// clang-format off
void export_SpikeReportWriter()
{

bp::class_<SpikeReportWriter, boost::noncopyable>(
    "SpikeReportWriter", bp::no_init)
    .def("__init__", bp::make_constructor(_initURI),
         DOXY_FN(brain::SpikeReportWriter::SpikeReportWriter(const brion::URI&)))
    .def("close", &SpikeReportWriter::close,
         DOXY_FN(brain::SpikeReportWriter::close))
    .def("write_spikes", &SpikeReportWriter_writeSpikes,
         (bp::arg("self"), bp::arg("spikes")),
         DOXY_FN(brain::SpikeReportWriter::writeSpikes(const Spikes&)))
    ;
}
// clang-format on
}
