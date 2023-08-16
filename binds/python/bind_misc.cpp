/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bind_misc.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/collection.h>
#include <morphio/enums.h>
#include <morphio/errorMessages.h>
#include <morphio/types.h>
#include <morphio/version.h>

#include "bind_enums.h"
#include "generated/docstrings.h"


namespace py = pybind11;

void bind_misc(py::module& m) {
    using namespace py::literals;

    m.def("set_maximum_warnings",
          &morphio::set_maximum_warnings,
          DOC(morphio, set_maximum_warnings));
    m.def("set_raise_warnings", &morphio::set_raise_warnings, DOC(morphio, set_raise_warnings));
    m.def("set_ignored_warning",
          static_cast<void (*)(morphio::Warning, bool)>(&morphio::set_ignored_warning),
          DOC(morphio, set_ignored_warning),
          "warning"_a,
          "ignore"_a = true);
    m.def("set_ignored_warning",
          static_cast<void (*)(const std::vector<morphio::Warning>&, bool)>(
              &morphio::set_ignored_warning),
          DOC(morphio, set_ignored_warning),
          "warning"_a,
          "ignore"_a = true);

    m.attr("version") = morphio::getVersionString();

    auto base = py::register_exception<morphio::MorphioError&>(m, "MorphioError");
    // base.ptr() signifies "inherits from"
    auto raw = py::register_exception<morphio::RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<morphio::UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<morphio::SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<morphio::IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<morphio::MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<morphio::MissingParentError&>(m, "MissingParentError", raw.ptr());
    py::register_exception<morphio::SectionBuilderError&>(m, "SectionBuilderError", raw.ptr());
    py::register_exception<morphio::WriterError&>(m, "WriterError", base.ptr());

    py::class_<morphio::Points>(m, "Points", py::buffer_protocol())
        .def_buffer([](morphio::Points& points) -> py::buffer_info {
            return py::buffer_info(points.data(),              /* Pointer to buffer */
                                   sizeof(morphio::floatType), /* Size of one scalar */
                                   py::format_descriptor<morphio::floatType>::format(), /* Python
                                                                              struct-style format
                                                                              descriptor */
                                   2, /* Number of dimensions */
                                   {static_cast<ssize_t>(points.size()),
                                    static_cast<ssize_t>(3)},       /* Buffer dimensions */
                                   {sizeof(morphio::floatType) * 3, /* Strides (in bytes) for each
                                                                       index */
                                    sizeof(morphio::floatType)});
        });

    py::class_<morphio::Property::Properties>(
        m, "Properties", "The higher level container structure is Property::Properties")
        .def_readwrite("point_level",
                       &morphio::Property::Properties::_pointLevel,
                       "Returns the structure that stores information at the point level")
        .def_readwrite("section_level",
                       &morphio::Property::Properties::_sectionLevel,
                       "Returns the structure that stores information at the section level")
        .def_readwrite("cell_level",
                       &morphio::Property::Properties::_cellLevel,
                       "Returns the structure that stores information at the cell level");

    py::class_<morphio::Property::PointLevel>(m, "PointLevel", DOC(morphio, Property, PointLevel))
        .def(py::init<>())
        .def(py::init<std::vector<morphio::Property::Point::Type>,
                      std::vector<morphio::Property::Diameter::Type>>(),
             "points"_a,
             "diameters"_a)
        .def(py::init<std::vector<morphio::Property::Point::Type>,
                      std::vector<morphio::Property::Diameter::Type>,
                      std::vector<morphio::Property::Perimeter::Type>>(),
             "points"_a,
             "diameters"_a,
             "perimeters"_a)
        .def_readwrite("points",
                       &morphio::Property::PointLevel::_points,
                       "Returns the list of point coordinates")
        .def_readwrite("perimeters",
                       &morphio::Property::PointLevel::_perimeters,
                       "Returns the list of perimeters")
        .def_readwrite("diameters",
                       &morphio::Property::PointLevel::_diameters,
                       "Returns the list of diameters");

    py::class_<morphio::Property::SectionLevel>(m,
                                                "SectionLevel",
                                                DOC(morphio, Property, SectionLevel))
        .def_readwrite("sections",
                       &morphio::Property::SectionLevel::_sections,
                       "Returns a list of [offset, parent section ID]")
        .def_readwrite("section_types",
                       &morphio::Property::SectionLevel::_sectionTypes,
                       "Returns the list of section types")
        .def_readwrite("children",
                       &morphio::Property::SectionLevel::_children,
                       "Returns a dictionary where key is a section ID "
                       "and value is the list of children section IDs");

    py::class_<morphio::Property::CellLevel>(m, "CellLevel", DOC(morphio, Property, CellLevel))
        .def_readwrite("cell_family",
                       &morphio::Property::CellLevel::_cellFamily,
                       "Returns the cell family (neuron or glia)")
        .def_readwrite("soma_type",
                       &morphio::Property::CellLevel::_somaType,
                       "Returns the soma type")
        .def_readwrite("version", &morphio::Property::CellLevel::_version, "Returns the version");

    py::class_<morphio::Property::Annotation>(m, "Annotation", DOC(morphio, Property, Annotation))
        .def_readwrite("type", &morphio::Property::Annotation::_type, "Returns the type")
        .def_readwrite("section_id",
                       &morphio::Property::Annotation::_sectionId,
                       "Returns the sectionId")
        .def_readwrite("line_number",
                       &morphio::Property::Annotation::_lineNumber,
                       "Returns the lineNumber")
        .def_readwrite("details", &morphio::Property::Annotation::_details, "Returns the details")
        .def_property_readonly(
            "points",
            [](morphio::Property::Annotation* a) { return a->_points._points; },
            "Returns the list of coordinates of annotated points")
        .def_property_readonly(
            "diameters",
            [](morphio::Property::Annotation* a) { return a->_points._diameters; },
            "Returns the list of diameters of annotated points")
        .def_property_readonly(
            "perimeters",
            [](morphio::Property::Annotation* a) { return a->_points._perimeters; },
            "Returns the list of perimeters of annotated points");

    py::class_<morphio::Property::Marker>(m,
                                          "Marker",
                                          "Container class for NeuroLucida extra Markers ")
        .def_property_readonly(
            "label",
            [](morphio::Property::Marker* marker) { return marker->_label; },
            "Returns the label")
        .def_property_readonly(
            "points",
            [](morphio::Property::Marker* marker) { return marker->_pointLevel._points; },
            "Returns the list of coordinates of the marker points")
        .def_property_readonly(
            "diameters",
            [](morphio::Property::Marker* marker) { return marker->_pointLevel._diameters; },
            "Returns the list of diameters of the marker points")
        .def_property_readonly(
            "section_id",
            [](morphio::Property::Marker* marker) { return marker->_sectionId; },
            "Returns the id of section that contains the marker");

    py::class_<morphio::Property::MitochondriaPointLevel>(
        m, "MitochondriaPointLevel", DOC(morphio, Property, MitochondriaPointLevel))
        .def(py::init<>())
        .def(py::init<std::vector<uint32_t>,
                      std::vector<morphio::floatType>,
                      std::vector<morphio::Property::Diameter::Type>>(),
             "neuronal_section_ids"_a,
             "distances_to_section_start"_a,
             "diameters"_a);

    py::class_<morphio::Property::DendriticSpine::PostSynapticDensity>(
        m, "PostSynapticDensity", DOC(morphio, Property, DendriticSpine, PostSynapticDensity))
        .def(py::init<>())
        .def(py::init<morphio::Property::DendriticSpine::SectionId_t,
                      morphio::Property::DendriticSpine::SegmentId_t,
                      morphio::Property::DendriticSpine::Offset_t>(),
             "section_id"_a,
             "segment_id"_a,
             "offset"_a)
        .def_readonly("section_id",
                      &morphio::Property::DendriticSpine::PostSynapticDensity::sectionId,
                      "Returns `sectionId` of post-synaptic density")
        .def_readonly("segment_id",
                      &morphio::Property::DendriticSpine::PostSynapticDensity::segmentId,
                      "Returns `segmentId` of post-synaptic density")
        .def_readonly("offset",
                      &morphio::Property::DendriticSpine::PostSynapticDensity::offset,
                      "Returns `offset` of post-synaptic density");

    py::class_<morphio::Collection>(m, "Collection", "A collection of morphologies")
        .def(py::init<std::string>(), "collection_path"_a)
        .def(py::init([](py::object arg) { return morphio::Collection(py::str(arg)); }),
             "collection_path"_a,
             "Create a collection from a Path-like object.")
        .def(py::init([](py::object arg, std::vector<std::string> extensions) {
                 return morphio::Collection(py::str(arg), std::move(extensions));
             }),
             "collection_path"_a,
             "extensions"_a,
             "Create a collection from a Path-like object.")
        .def(
            "load",
            [](morphio::Collection* collection,
               const std::string& morph_name,
               unsigned int options,
               bool is_mutable) -> py::object {
                if (is_mutable) {
                    return py::cast(
                        collection->load<morphio::mut::Morphology>(morph_name, options));
                } else {
                    return py::cast(collection->load<morphio::Morphology>(morph_name, options));
                }
            },
            "morph_name"_a,
            "options"_a = morphio::enums::Option::NO_MODIFIER,
            "mutable"_a = false,
            "Load the morphology named 'morph_name' form the collection.")
        .def(
            "load_unordered",
            [](morphio::Collection* collection,
               std::vector<std::string> morphology_names,
               unsigned int options,
               bool is_mutable) -> py::object {
                if (is_mutable) {
                    return py::cast(
                        collection->load_unordered<morphio::mut::Morphology>(morphology_names,
                                                                             options));
                } else {
                    return py::cast(
                        collection->load_unordered<morphio::Morphology>(morphology_names, options));
                }
            },
            "morphology_names"_a,
            "options"_a = morphio::enums::Option::NO_MODIFIER,
            "mutable"_a = false,
            R"(Create an iterable of loop index and morphology.

When reading from containers, the order in which morphologies are read can
have a large impact on the overall time to load those morphologies.

This iterator provides means of reordering loops to optimize the access
pattern. Loops such as the following

    for k, morph_name in enumerate(morphology_names):
        morph = collection.load(morphology_names[k])
        f(k, morph)

can be replaced with

    for k, morph in collection.load_unordered(morphology_names):
      assert collection.load(morphology_names[k]) == morph
      f(k, morph)

The order in which the morphologies are returned in unspecified, but the
loop index `k` can be used to retrieve the correct state corresponding to
iteration `k` of the original loop.

The iterable returned by `Collection.load_unordered` should only be used while
`collection` is valid, e.g. within its context or before calling
`Collection.close`.

Note: This API is 'experimental', meaning it might change in the future.
)")

        .def("argsort",
             &morphio::Collection::argsort,
             "morphology_names"_a,
             R"(Argsort `morphology_names` by optimal access order.

Note: This API is 'experimental', meaning it might change in the future.
)")
        .def("__enter__", [](morphio::Collection* collection) { return collection; })
        .def("__exit__",
             [](morphio::Collection* collection,
                const py::object&,
                const py::object&,
                const py::object&) { collection->close(); })
        .def("close", &morphio::Collection::close);

    py::class_<morphio::LoadUnordered<morphio::Morphology>>(
        m, "LoadImmutableUnordered", "An iterable of immutable morphologies.")
        .def(
            "__iter__",
            [](const morphio::LoadUnordered<morphio::Morphology>& iterable) {
                return py::make_iterator(iterable.begin(), iterable.end());
            },
            // Bind the lifetime of the `morphio::LoadUnordered` (1) to the
            // lifetime of the returned iterator (0).
            py::keep_alive<0, 1>());

    py::class_<morphio::LoadUnordered<morphio::mut::Morphology>>(
        m, "LoadMutableUnordered", "An iterable of mutable morphologies.")
        .def(
            "__iter__",
            [](const morphio::LoadUnordered<morphio::mut::Morphology>& iterable) {
                return py::make_iterator(iterable.begin(), iterable.end());
            },
            // Bind the lifetime of the `morphio::LoadUnordered` (1) to the
            // lifetime of the returned iterator (0).
            py::keep_alive<0, 1>());
}
