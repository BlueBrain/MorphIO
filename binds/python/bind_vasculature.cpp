/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bind_vasculature.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/vasc/section.h>
#include <morphio/vasc/vasculature.h>

#include <memory>  // std::make_unique

#include "bindings_utils.h"

namespace py = pybind11;

void bind_vasculature(py::module& m) {
    using namespace py::literals;

    py::class_<morphio::vasculature::Vasculature>(m,
                                                  "Vasculature",
                                                  "Class representing a Vasculature")
        .def(py::init<const std::string&>(), "filename"_a)
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::vasculature::Vasculature>(py::str(arg));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python object that implements __repr__ "
             "or __str__")

        .def_property_readonly(
            "sections",
            &morphio::vasculature::Vasculature::sections,
            "Returns a vector containing all sections objects\n\n"
            "Note: To select sections by ID use: VasculatureMorphology::section(id)")

        .def("section",
             &morphio::vasculature::Vasculature::section,
             "Returns the Section with the given id\n"
             "throw RawDataError if the id is out of range",
             "section_id"_a)

        .def_property_readonly(
            "section_offsets",
            [](morphio::vasculature::Vasculature& vasculature) {
                return as_pyarray(vasculature.sectionOffsets());
            },
            "Returns a list with offsets to access data of a specific section in the points\n"
            "and diameters arrays.\n"
            "\n"
            "Example: accessing diameters of n'th section will be located in the DIAMETERS\n"
            "array from DIAMETERS[sectionOffsets(n)] to DIAMETERS[sectionOffsets(n+1)-1]\n"
            "\n"
            "Note: for convenience, the last point of this array is the points array size\n"
            "so that the above example works also for the last section.")

        // Property accessors
        .def_property_readonly(
            "points",
            [](morphio::vasculature::Vasculature* morpho) {
                return py::array(static_cast<py::ssize_t>(morpho->points().size()),
                                 morpho->points().data());
            },
            "Returns a list with all points from all sections")

        .def_property_readonly(
            "n_points",
            [](const morphio::vasculature::Vasculature& obj) { return obj.points().size(); },
            "Returns the number of points from all sections")

        .def_property_readonly(
            "diameters",
            [](morphio::vasculature::Vasculature* morpho) {
                auto diameters = morpho->diameters();
                return py::array(static_cast<py::ssize_t>(diameters.size()), diameters.data());
            },
            "Returns a list with all diameters from all sections")
        .def_property_readonly(
            "section_types",
            [](morphio::vasculature::Vasculature* obj) {
                auto data = obj->sectionTypes();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a vector with the section type of every section")

        .def_property_readonly(
            "section_connectivity",
            [](morphio::vasculature::Vasculature* morpho) {
                return py::array(static_cast<py::ssize_t>(morpho->sectionConnectivity().size()),
                                 morpho->sectionConnectivity().data());
            },
            "Returns a 2D array of the section connectivity")

        // Iterators
        .def(
            "iter",
            [](morphio::vasculature::Vasculature* morpho) {
                return py::make_iterator(morpho->begin(), morpho->end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Iterate on all sections of the graph");


    py::class_<morphio::vasculature::Section>(m,
                                              "Section",
                                              "Class representing a mutable Vasculature Section")
        .def("__str__",
             [](const morphio::vasculature::Section& section) {
                 std::stringstream ss;
                 ss << section;
                 return ss.str();
             })

        // Topology-related member functions
        .def_property_readonly("predecessors",
                               &morphio::vasculature::Section::predecessors,
                               "Returns the predecessors section of this section")
        .def_property_readonly("successors",
                               &morphio::vasculature::Section::successors,
                               "Returns the successors section of this section")
        .def_property_readonly("neighbors",
                               &morphio::vasculature::Section::neighbors,
                               "Returns the neighbors section of this section")

        // Property-related accessors
        .def_property_readonly("id",
                               &morphio::vasculature::Section::id,
                               "Returns the section ID\n"
                               "The section ID can be used to query sections via "
                               "VasculatureSection::section(uint32_t id)")
        .def_property_readonly("type",
                               &morphio::vasculature::Section::type,
                               "Returns the morphological type of this section")
        .def_property_readonly(
            "points",
            [](morphio::vasculature::Section* section) {
                return span_array_to_ndarray(section->points());
            },
            "Returns list of section's point coordinates")

        .def_property_readonly(
            "n_points",
            [](const morphio::vasculature::Section& section) { return section.points().size(); },
            "Returns the number of point in section")

        .def_property_readonly(
            "diameters",
            [](morphio::vasculature::Section* section) {
                return span_to_ndarray(section->diameters());
            },
            "Returns list of section's point diameters")

        // Iterators
        .def(
            "iter",
            [](morphio::vasculature::Section* section) {
                return py::make_iterator(section->begin(), section->end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator\n");
}
