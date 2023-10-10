/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bind_immutable.h"

#include <pybind11/iostream.h>  // py::add_ostream_redirect
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/dendritic_spine.h>
#include <morphio/endoplasmic_reticulum.h>
#include <morphio/enums.h>
#include <morphio/glial_cell.h>
#include <morphio/mut/dendritic_spine.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/soma.h>
#include <morphio/types.h>

#include <memory>  // std::make_unique

#include "bind_enums.h"
#include "bindings_utils.h"
#include "generated/docstrings.h"


namespace py = pybind11;
using namespace py::literals;

void bind_morphology(py::module& m);
void bind_glialcell(py::module& m);
void bind_mitochondria(py::module& m);
void bind_mitosection(py::module& m);
void bind_section(py::module& m);
void bind_soma(py::module& m);
void bind_endoplasmic_reticulum(py::module& m);
void bind_dendritic_spine(py::module& m);

void bind_immutable(py::module& m) {
    // http://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=iostream#capturing-standard-output-from-ostream
    py::add_ostream_redirect(m, "ostream_redirect");
    bind_morphology(m);
    bind_glialcell(m);
    bind_mitochondria(m);
    bind_mitosection(m);
    bind_section(m);
    bind_soma(m);
    bind_endoplasmic_reticulum(m);
    bind_dendritic_spine(m);
}

void bind_morphology(py::module& m) {
#define D(x) DOC(morphio, Morphology, x)
    py::class_<morphio::Morphology>(m, "Morphology", DOC(morphio, Morphology))
        .def(py::init<const std::string&, unsigned int>(),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const std::string&, const std::string&, unsigned int>(),
             "filename"_a,
             "extension"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<morphio::mut::Morphology&>())
        .def(py::init([](py::object arg, unsigned int options) {
                 return std::make_unique<morphio::Morphology>(py::str(arg), options);
             }),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER,
             "Additional Ctor that accepts as filename any python object that implements __repr__ "
             "or __str__")
        .def("as_mutable",
             [](const morphio::Morphology* morph) { return morphio::mut::Morphology(*morph); })

        // Cell sub-parts accessors
        .def_property_readonly("soma", &morphio::Morphology::soma, D(soma))
        .def_property_readonly("mitochondria", &morphio::Morphology::mitochondria, D(mitochondria))
        .def_property_readonly("annotations", &morphio::Morphology::annotations, D(annotations))
        .def_property_readonly("markers", &morphio::Morphology::markers, D(markers))
        .def_property_readonly("endoplasmic_reticulum",
                               &morphio::Morphology::endoplasmicReticulum,
                               D(endoplasmicReticulum))
        .def_property_readonly("root_sections", &morphio::Morphology::rootSections, D(rootSections))
        .def_property_readonly("sections", &morphio::Morphology::sections, D(section))
        .def("section", &morphio::Morphology::section, D(section), "section_id"_a)

        // Property accessors
        .def_property_readonly(
            "points",
            [](morphio::Morphology* morpho) {
                const auto& data = morpho->points();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            D(points))
        .def_property_readonly(
            "n_points",
            [](const morphio::Morphology& obj) { return obj.points().size(); },
            "Returns the number of points from all sections (soma points are not included)")
        .def_property_readonly(
            "diameters",
            [](const morphio::Morphology& morpho) {
                const auto& data = morpho.diameters();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            D(diameters))
        .def_property_readonly(
            "perimeters",
            [](const morphio::Morphology& obj) {
                const auto& data = obj.perimeters();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            D(perimeters))
        .def_property_readonly(
            "section_offsets",
            [](const morphio::Morphology& morpho) { return as_pyarray(morpho.sectionOffsets()); },
            D(sectionOffsets))
        .def_property_readonly(
            "section_types",
            [](const morphio::Morphology& morph) {
                const auto& data = morph.sectionTypes();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            D(sectionTypes))
        .def_property_readonly("connectivity", &morphio::Morphology::connectivity, D(connectivity))
        .def_property_readonly("soma_type", &morphio::Morphology::somaType, D(somaType))
        .def_property_readonly("cell_family", &morphio::Morphology::cellFamily, D(cellFamily))
        .def_property_readonly("version", &morphio::Morphology::version, D(version))

        // Iterators
        .def(
            "iter",
            [](morphio::Morphology* morpho, IterType type) {
                switch (type) {
                case IterType::DEPTH_FIRST:
                    return py::make_iterator(morpho->depth_begin(), morpho->depth_end());
                case IterType::BREADTH_FIRST:
                    return py::make_iterator(morpho->breadth_begin(), morpho->breadth_end());
                case IterType::UPSTREAM:
                default:
                    throw morphio::MorphioError(
                        "Only iteration types depth_first and breadth_first are supported");
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator that runs successively on every neurite\n"
            "\n"
            "iter_type controls the order of iteration on sections of a given neurite. 2 values "
            "can be passed:\n"
            "\n"
            "- ``morphio.IterType.depth_first`` (default)\n"
            "- ``morphio.IterType.breadth_first``\n",
            "iter_type"_a = IterType::DEPTH_FIRST);
#undef D
}

void bind_glialcell(py::module& m) {
    py::class_<morphio::GlialCell, morphio::Morphology>(m, "GlialCell", DOC(morphio, GlialCell))
        .def(py::init<const std::string&>())
        .def(py::init(
                 [](py::object arg) { return std::make_unique<morphio::GlialCell>(py::str(arg)); }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python object that implements __repr__ "
             "or __str__");
}

void bind_mitochondria(py::module& m) {
    py::class_<morphio::Mitochondria>(m, "Mitochondria", DOC(morphio, Mitochondria))
        .def("section",
             &morphio::Mitochondria::section,
             DOC(morphio, Mitochondria, section),
             "section_id"_a)
        .def_property_readonly("sections",
                               &morphio::Mitochondria::sections,
                               DOC(morphio, Mitochondria, sections))
        .def_property_readonly("root_sections",
                               &morphio::Mitochondria::rootSections,
                               DOC(morphio, Mitochondria, rootSections));
}

void bind_mitosection(py::module& m) {
    using morphio::MitoSection;
    py::class_<MitoSection>(m, "MitoSection", "Class representing a Mitochondrial Section")
        // Topology-related member functions
        .def_property_readonly("parent", &MitoSection::parent, DOC(morphio, SectionBase, parent))
        .def_property_readonly("is_root", &MitoSection::isRoot, DOC(morphio, SectionBase, isRoot))
        .def_property_readonly("children",
                               &MitoSection::children,
                               DOC(morphio, SectionBase, children))

        // Property-related accesors
        .def_property_readonly("id", &MitoSection::id, DOC(morphio, SectionBase, id))
        .def_property_readonly(
            "neurite_section_ids",
            [](MitoSection* section) { return span_to_ndarray(section->neuriteSectionIds()); },
            DOC(morphio, MitoSection, neuriteSectionIds))
        .def_property_readonly(
            "diameters",
            [](MitoSection* section) { return span_to_ndarray(section->diameters()); },
            DOC(morphio, MitoSection, diameters))
        .def_property_readonly(
            "relative_path_lengths",
            [](MitoSection* section) { return span_to_ndarray(section->relativePathLengths()); },
            DOC(morphio, MitoSection, relativePathLengths))
        .def("has_same_shape",
             &MitoSection::hasSameShape,
             DOC(morphio, MitoSection, relativePathLengths))

        // Iterators
        .def(
            "iter",
            [](MitoSection* section, IterType type) {
                switch (type) {
                case IterType::DEPTH_FIRST:
                    return py::make_iterator(section->depth_begin(), section->depth_end());
                case IterType::BREADTH_FIRST:
                    return py::make_iterator(section->breadth_begin(), section->breadth_end());
                case IterType::UPSTREAM:
                    return py::make_iterator(section->upstream_begin(), section->upstream_end());
                default:
                    throw morphio::MorphioError(
                        "Only iteration types depth_first, breadth_first and "
                        "upstream are supported");
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "iter_type"_a = IterType::DEPTH_FIRST);
}

void bind_section(py::module& m) {
#define D(x) DOC(morphio, Section, x)
#define DB(x) DOC(morphio, SectionBase, x)
    using morphio::Section;
    py::class_<Section>(m, "Section", "Class representing a Section")
        .def("__str__",
             [](const Section& section) {
                 std::stringstream ss;
                 ss << section;
                 return ss.str();
             })
        .def_property_readonly("parent", &Section::parent, DB(parent))
        .def_property_readonly("is_root", &Section::isRoot, DB(isRoot))
        .def_property_readonly("children", &Section::children, DB(children))
        .def_property_readonly("id", &Section::id, DB(id))
        .def_property_readonly("type", &Section::type, D(type))
        .def_property_readonly(
            "points",
            [](Section* section) { return span_array_to_ndarray(section->points()); },
            D(points))
        .def_property_readonly(
            "n_points",
            [](const Section& section) { return section.points().size(); },
            "Returns the number of points in section")
        .def_property_readonly(
            "diameters",
            [](Section* section) { return span_to_ndarray(section->diameters()); },
            D(diameters))
        .def_property_readonly(
            "perimeters",
            [](Section* section) { return span_to_ndarray(section->perimeters()); },
            D(perimeters))
        .def("is_heterogeneous",
             &Section::isHeterogeneous,
             D(isHeterogeneous),
             py::arg("downstream") = true)
        .def("has_same_shape", &Section::hasSameShape, D(hasSameShape))
        .def(
            "iter",
            [](Section* section, IterType type) {
                switch (type) {
                case IterType::DEPTH_FIRST:
                    return py::make_iterator(section->depth_begin(), section->depth_end());
                case IterType::BREADTH_FIRST:
                    return py::make_iterator(section->breadth_begin(), section->breadth_end());
                case IterType::UPSTREAM:
                    return py::make_iterator(section->upstream_begin(), section->upstream_end());
                default:
                    throw morphio::MorphioError(
                        "Only iteration types depth_first, breadth_first and "
                        "upstream are supported");
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator\n"
            "\n"
            "iter_type controls the iteration order. 3 values can be passed:\n"
            "\n"
            "- ``morphio.IterType.depth_first`` (default)\n"
            "- ``morphio.IterType.breadth_first``\n"
            "- ``morphio.IterType.upstream``\n",
            "iter_type"_a = IterType::DEPTH_FIRST);
#undef D
}

void bind_soma(py::module& m) {
    py::class_<morphio::Soma>(m, "Soma", DOC(morphio, Soma))
        .def(py::init<const morphio::Soma&>())
        .def_property_readonly(
            "points",
            [](morphio::Soma* soma) { return span_array_to_ndarray(soma->points()); },
            DOC(morphio, Soma, points))
        .def_property_readonly(
            "diameters",
            [](morphio::Soma* soma) { return span_to_ndarray(soma->diameters()); },
            DOC(morphio, Soma, diameters))

        .def_property_readonly(
            "center",
            [](morphio::Soma* soma) { return py::array(3, soma->center().data()); },
            DOC(morphio, Soma, center))
        .def_property_readonly("max_distance",
                               &morphio::Soma::maxDistance,
                               DOC(morphio, Soma, maxDistance))
        .def_property_readonly("type", &morphio::Soma::type, DOC(morphio, Soma, type))
        .def_property_readonly("surface", &morphio::Soma::surface, DOC(morphio, Soma, surface));
}

void bind_endoplasmic_reticulum(py::module& m) {
    py::class_<morphio::EndoplasmicReticulum>(m,
                                              "EndoplasmicReticulum",
                                              DOC(morphio, EndoplasmicReticulum))
        .def_property_readonly("section_indices",
                               &morphio::EndoplasmicReticulum::sectionIndices,
                               DOC(morphio, EndoplasmicReticulum, sectionIndices))
        .def_property_readonly("volumes",
                               &morphio::EndoplasmicReticulum::volumes,
                               DOC(morphio, EndoplasmicReticulum, volumes))
        .def_property_readonly("surface_areas",
                               &morphio::EndoplasmicReticulum::surfaceAreas,
                               DOC(morphio, EndoplasmicReticulum, surfaceAreas))
        .def_property_readonly("filament_counts",
                               &morphio::EndoplasmicReticulum::filamentCounts,
                               DOC(morphio, EndoplasmicReticulum, filamentCounts));
}

void bind_dendritic_spine(py::module& m) {
    py::class_<morphio::DendriticSpine, morphio::Morphology>(m,
                                                             "DendriticSpine",
                                                             DOC(morphio, DendriticSpine))
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::DendriticSpine>(py::str(arg));
             }),
             "filename"_a)
        .def_property_readonly("root_sections",
                               &morphio::DendriticSpine::rootSections,
                               DOC(morphio, Morphology, rootSections))
        .def_property_readonly("sections",
                               &morphio::DendriticSpine::sections,
                               DOC(morphio, Morphology, sections))
        .def("section",
             &morphio::DendriticSpine::section,
             DOC(morphio, Morphology, section),
             "section_id"_a)
        .def_property_readonly(
            "points",
            [](morphio::DendriticSpine* morpho) {
                const auto& data = morpho->points();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            DOC(morphio, Morphology, points))
        .def_property_readonly(
            "diameters",
            [](const morphio::DendriticSpine& morpho) {
                const auto& data = morpho.diameters();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            DOC(morphio, Morphology, diameters))
        .def_property_readonly(
            "section_offsets",
            [](const morphio::DendriticSpine& morpho) {
                return as_pyarray(morpho.sectionOffsets());
            },
            DOC(morphio, Morphology, sectionOffsets))
        .def_property_readonly(
            "section_types",
            [](const morphio::DendriticSpine& morph) {
                const auto& data = morph.sectionTypes();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            DOC(morphio, Morphology, sectionTypes))
        .def_property_readonly("connectivity",
                               &morphio::DendriticSpine::connectivity,
                               DOC(morphio, Morphology, connectivity))
        .def_property_readonly("cell_family",
                               &morphio::DendriticSpine::cellFamily,
                               DOC(morphio, Morphology, cellFamily))
        .def_property_readonly("post_synaptic_density",
                               &morphio::DendriticSpine::postSynapticDensity,
                               DOC(morphio, DendriticSpine, postSynapticDensity))
        .def_property_readonly("version",
                               &morphio::DendriticSpine::version,
                               DOC(morphio, Morphology, version))

        // Iterators
        .def(
            "iter",
            [](morphio::DendriticSpine* morpho, IterType type) {
                switch (type) {
                case IterType::DEPTH_FIRST:
                    return py::make_iterator(morpho->depth_begin(), morpho->depth_end());
                case IterType::BREADTH_FIRST:
                    return py::make_iterator(morpho->breadth_begin(), morpho->breadth_end());
                case IterType::UPSTREAM:
                default:
                    throw morphio::MorphioError(
                        "Only iteration types depth_first and breadth_first are supported");
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator that runs successively on every neurite\n"
            "\n"
            "iter_type controls the order of iteration on sections of a given neurite. 2 values "
            "can be passed:\n"
            "\n"
            "- ``morphio.IterType.depth_first`` (default)\n"
            "- ``morphio.IterType.breadth_first``\n",
            "iter_type"_a = IterType::DEPTH_FIRST)
        .def(
            "write",
            [](morphio::mut::DendriticSpine* morph, py::object arg) { morph->write(py::str(arg)); },
            "filename"_a);
}
