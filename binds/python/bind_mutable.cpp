/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bind_mutable.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mut/dendritic_spine.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>

#include <memory>  // std::make_unique

#include "bind_enums.h"
#include "bindings_utils.h"
#include "generated/docstrings.h"

namespace py = pybind11;
using namespace py::literals;

void bind_mut_morphology(py::module& m);
void bind_mut_glialcell(py::module& m);
void bind_mut_mitochondria(py::module& m);
void bind_mut_mitosection(py::module& m);
void bind_mut_section(py::module& m);
void bind_mut_soma(py::module& m);
void bind_mut_endoplasmic_reticulum(py::module& m);
void bind_mut_dendritic_spine(py::module& m);

void bind_mutable(py::module& m) {
    bind_mut_morphology(m);
    bind_mut_glialcell(m);
    bind_mut_mitochondria(m);
    bind_mut_mitosection(m);
    bind_mut_section(m);
    bind_mut_soma(m);
    bind_mut_endoplasmic_reticulum(m);
    bind_mut_dendritic_spine(m);
}

void bind_mut_morphology(py::module& m) {
#define D(x) DOC(morphio, mut, Morphology, x)
    using morphio::mut::Morphology;

    py::class_<Morphology>(m, "Morphology", "Class representing a mutable Morphology")
        .def(py::init<>())
        .def(py::init<const std::string&, unsigned int>(),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const morphio::Morphology&, unsigned int>(),
             "morphology"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const Morphology&, unsigned int>(),
             "morphology"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init([](py::object arg, unsigned int options) {
                 return std::make_unique<Morphology>(py::str(arg), options);
             }),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__")

        // Cell sub-part accessors
        .def_property_readonly("sections", &Morphology::sections, D(sections))
        .def_property_readonly("root_sections",
                               &Morphology::rootSections,
                               D(rootSections),
                               py::return_value_policy::reference)
        .def_property_readonly("soma",
                               static_cast<std::shared_ptr<morphio::mut::Soma>& (Morphology::*) ()>(
                                   &Morphology::soma),
                               D(soma))
        .def_property_readonly("mitochondria",
                               static_cast<morphio::mut::Mitochondria& (Morphology::*) ()>(
                                   &Morphology::mitochondria),
                               D(mitochondria))
        .def_property_readonly("endoplasmic_reticulum",
                               static_cast<morphio::mut::EndoplasmicReticulum& (Morphology::*) ()>(
                                   &Morphology::endoplasmicReticulum),
                               D(endoplasmicReticulum))
        .def_property_readonly("annotations", &Morphology::annotations, D(annotations))
        .def_property_readonly("markers", &Morphology::markers, D(markers))
        .def("section", &Morphology::section, D(section), "section_id"_a)
        .def("build_read_only", &Morphology::buildReadOnly, D(buildReadOnly))
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 Morphology::*)(const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &Morphology::appendRootSection),
             D(appendRootSection),
             "point_level_properties"_a,
             "section_type"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 Morphology::*)(const morphio::Section&, bool)>(&Morphology::appendRootSection),
             D(appendRootSection),
             "immutable_section"_a,
             "recursive"_a = false)

        .def("delete_section",
             &Morphology::deleteSection,
             D(deleteSection),
             "section"_a,
             "recursive"_a = true)
        .def("as_immutable", [](const Morphology* morph) { return morphio::Morphology(*morph); })
        .def_property_readonly("connectivity", &Morphology::connectivity, D(connectivity))
        .def_property_readonly("cell_family", &Morphology::cellFamily, D(cellFamily))
        .def_property_readonly("soma_type", &Morphology::somaType, D(somaType))
        .def_property_readonly("version", &Morphology::version, D(version))
        .def("remove_unifurcations",
             static_cast<void (Morphology::*)()>(&Morphology::removeUnifurcations),
             D(removeUnifurcations))
        .def(
            "write",
            [](Morphology* morph, py::object arg) { morph->write(py::str(arg)); },
            D(write),
            "filename"_a)

        // Iterators
        .def(
            "iter",
            [](Morphology* morph, IterType type) {
                switch (type) {
                case IterType::DEPTH_FIRST:
                    return py::make_iterator(morph->depth_begin(), morph->depth_end());
                case IterType::BREADTH_FIRST:
                    return py::make_iterator(morph->breadth_begin(), morph->breadth_end());
                case IterType::UPSTREAM:
                default:
                    throw morphio::MorphioError(
                        "Only iteration types depth_first and "
                        "breadth_first are supported");
                }
            },
            py::keep_alive<0, 1>(), /* Essential: keep object alive while iterator exists */
            "Section iterator that runs successively on every neurite\n"
            "\n"
            "iter_type controls the order of iteration on sections of "
            "a given neurite. 2 values can be passed:\n"
            "\n"
            "- ``morphio.IterType.depth_first`` (default)\n"
            "- ``morphio.IterType.breadth_first``\n",
            "iter_type"_a = IterType::DEPTH_FIRST)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 Morphology::*)(const std::shared_ptr<morphio::mut::Section>&, bool)>(
                 &Morphology::appendRootSection),
             D(appendRootSection),
             "mutable_section"_a,
             "recursive"_a = false);
#undef D
}

void bind_mut_glialcell(py::module& m) {
    py::class_<morphio::mut::GlialCell, morphio::mut::Morphology>(m,
                                                                  "GlialCell",
                                                                  DOC(morphio, mut, GlialCell))
        .def(py::init<>())
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::mut::GlialCell>(py::str(arg));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__");
}

void bind_mut_mitochondria(py::module& m) {
#define D(x) DOC(morphio, mut, Mitochondria, x)
    using morphio::mut::Mitochondria;
    using morphio::mut::MitoSection;
    py::class_<Mitochondria>(m, "Mitochondria", DOC(morphio, mut, Mitochondria))
        .def(py::init<>())
        .def_property_readonly("root_sections",
                               &Mitochondria::rootSections,
                               D(rootSections),
                               py::return_value_policy::reference)
        .def_property_readonly("sections", &Mitochondria::sections, D(sections))
        .def("is_root", &Mitochondria::isRoot, D(isRoot), "section_id"_a)
        .def("parent", &Mitochondria::parent, D(parent), "section_id"_a)
        .def("children", &Mitochondria::children, D(children), "section_id"_a)
        .def("section", &Mitochondria::section, D(section), "section_id"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<MitoSection> (Mitochondria::*)(
                 const morphio::Property::MitochondriaPointLevel&)>(
                 &Mitochondria::appendRootSection),
             D(appendRootSection),
             "point_level_properties"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<MitoSection> (Mitochondria::*)(const morphio::MitoSection&,
                                                                        bool recursive)>(
                 &Mitochondria::appendRootSection),
             D(appendRootSection_2),
             "immutable_section"_a,
             "recursive"_a = true)
        .def("append_root_section",
             static_cast<std::shared_ptr<MitoSection> (
                 Mitochondria::*)(const std::shared_ptr<MitoSection>&, bool recursive)>(
                 &Mitochondria::appendRootSection),
             D(appendRootSection_2),
             "section"_a,
             "recursive"_a = true)
        .def(
            "depth_begin",
            [](Mitochondria* morph, std::shared_ptr<MitoSection> section) {
                return py::make_iterator(morph->depth_begin(section), morph->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            D(depth_begin),
            "section_id"_a = -1)
        .def(
            "breadth_begin",
            [](Mitochondria* morph, std::shared_ptr<MitoSection> section) {
                return py::make_iterator(morph->breadth_begin(section), morph->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            D(breadth_begin),
            "section_id"_a = -1)
        .def(
            "upstream_begin",
            [](Mitochondria* morph, std::shared_ptr<MitoSection> section) {
                return py::make_iterator(morph->upstream_begin(section), morph->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            D(upstream_begin),
            "section_id"_a = -1);
#undef D
}

void bind_mut_mitosection(py::module& m) {
    using morphio::mut::MitoSection;
    using mitosection_floats_f = std::vector<morphio::floatType>& (MitoSection::*) ();
    using mitosection_ints_f = std::vector<uint32_t>& (MitoSection::*) ();

    py::class_<MitoSection, std::shared_ptr<MitoSection>>(m,
                                                          "MitoSection",
                                                          DOC(morphio, mut, MitoSection))
        .def_property_readonly("id", &MitoSection::id, "Return the section ID")
        .def_property(
            "diameters",
            static_cast<mitosection_floats_f>(&MitoSection::diameters),
            [](MitoSection* section, const std::vector<morphio::floatType>& _diameters) {
                section->diameters() = _diameters;
            },
            "Returns the diameters of all points of this section")
        .def_property(
            "relative_path_lengths",
            static_cast<mitosection_floats_f>(&MitoSection::pathLengths),
            [](MitoSection* section, const std::vector<morphio::floatType>& _pathLengths) {
                section->pathLengths() = _pathLengths;
            },
            "Returns the relative distance (between 0 and 1)\n"
            "between the start of the neuronal section and each point\n"
            "of this mitochondrial section")
        .def_property(
            "neurite_section_ids",
            static_cast<mitosection_ints_f>(&MitoSection::neuriteSectionIds),
            [](MitoSection* section, const std::vector<uint32_t>& _neuriteSectionIds) {
                section->neuriteSectionIds() = _neuriteSectionIds;
            },
            "Returns the neurite section Ids of all points of this section")
        .def("has_same_shape",
             &MitoSection::hasSameShape,
             DOC(morphio, mut, MitoSection, hasSameShape))
        .def("append_section",
             static_cast<std::shared_ptr<MitoSection> (MitoSection::*)(
                 const morphio::Property::MitochondriaPointLevel&)>(&MitoSection::appendSection),
             DOC(morphio, mut, MitoSection, appendSection),
             "point_level_properties"_a)
        .def("append_section",
             static_cast<std::shared_ptr<MitoSection> (MitoSection::*)(
                 const std::shared_ptr<MitoSection>&, bool)>(&MitoSection::appendSection),
             DOC(morphio, mut, MitoSection, appendSection_2),
             "section"_a,
             "recursive"_a = false)
        .def("append_section",
             static_cast<std::shared_ptr<MitoSection> (
                 MitoSection::*)(const morphio::MitoSection&, bool)>(&MitoSection::appendSection),
             DOC(morphio, mut, MitoSection, appendSection_2),
             "immutable_section"_a,
             "recursive"_a = false);
}

void bind_mut_section(py::module& m) {
#define D(x) DOC(morphio, mut, Section, x)
    py::class_<morphio::mut::Section, std::shared_ptr<morphio::mut::Section>>(
        m, "Section", "Class representing a mutable Section")
        .def("__str__",
             [](const morphio::mut::Section& section) {
                 std::stringstream ss;
                 ss << section;
                 return ss.str();
             })
        .def_property_readonly("id", &morphio::mut::Section::id, D(id))
        .def_property(
            "type",
            static_cast<const morphio::SectionType& (morphio::mut::Section::*) () const>(
                &morphio::mut::Section::type),
            [](morphio::mut::Section* section, morphio::SectionType _type) {
                section->type() = _type;
            },
            D(type))
        .def_property(
            "points",
            [](morphio::mut::Section* section) {
                return py::array(static_cast<py::ssize_t>(section->points().size()),
                                 section->points().data());
            },
            [](morphio::mut::Section* section, const py::array_t<morphio::floatType>& _points) {
                section->points() = array_to_points(_points);
            },
            D(points))
        .def_property(
            "diameters",
            [](morphio::mut::Section* section) {
                return py::array(static_cast<py::ssize_t>(section->diameters().size()),
                                 section->diameters().data());
            },
            [](morphio::mut::Section* section, py::array_t<morphio::floatType> _diameters) {
                section->diameters() = _diameters.cast<std::vector<morphio::floatType>>();
            },
            D(diameters))
        .def_property(
            "perimeters",
            [](morphio::mut::Section* section) {
                return py::array(static_cast<py::ssize_t>(section->perimeters().size()),
                                 section->perimeters().data());
            },
            [](morphio::mut::Section* section, py::array_t<morphio::floatType> _perimeters) {
                section->perimeters() = _perimeters.cast<std::vector<morphio::floatType>>();
            },
            D(perimeters))
        .def_property_readonly("is_root", &morphio::mut::Section::isRoot, D(isRoot))
        .def_property_readonly("parent", &morphio::mut::Section::parent, D(parent))
        .def_property_readonly("children", &morphio::mut::Section::children, D(children))
        .def("is_heterogeneous",
             &morphio::mut::Section::isHeterogeneous,
             D(isHeterogeneous),
             py::arg("downstream") = true)
        .def("has_same_shape", &morphio::mut::Section::hasSameShape, D(hasSameShape))

        // Iterators
        .def(
            "iter",
            [](morphio::mut::Section* section, IterType type) {
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
            "iter_type"_a = IterType::DEPTH_FIRST)

        // Editing

        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::Section::*)(
                 const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &morphio::mut::Section::appendSection),
             D(appendSection),
             "point_level_properties"_a,
             "section_type"_a = morphio::SectionType::SECTION_UNDEFINED)
        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::Section::*)(
                 const morphio::Section&, bool)>(&morphio::mut::Section::appendSection),
             D(appendSection_2),
             "immutable_section"_a,
             "recursive"_a = false)
        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 morphio::mut::Section::*)(std::shared_ptr<morphio::mut::Section>, bool)>(
                 &morphio::mut::Section::appendSection),
             D(appendSection_2),
             "mutable_section"_a,
             "recursive"_a = false);
#undef D
}

void bind_mut_soma(py::module& m) {
#define D(x) DOC(morphio, mut, Soma, x)
    py::class_<morphio::mut::Soma, std::shared_ptr<morphio::mut::Soma>>(m,
                                                                        "Soma",
                                                                        DOC(morphio, mut, Soma))
        .def(py::init<const morphio::Property::PointLevel&>())
        .def_property(
            "points",
            [](morphio::mut::Soma* soma) {
                return py::array(static_cast<py::ssize_t>(soma->points().size()),
                                 soma->points().data());
            },
            [](morphio::mut::Soma* soma, const py::array_t<morphio::floatType>& _points) {
                soma->points() = array_to_points(_points);
            },
            D(points))
        .def_property(
            "diameters",
            [](morphio::mut::Soma* soma) {
                return py::array(static_cast<py::ssize_t>(soma->diameters().size()),
                                 soma->diameters().data());
            },
            [](morphio::mut::Soma* soma, const py::array_t<morphio::floatType>& _diameters) {
                soma->diameters() = _diameters.cast<std::vector<morphio::floatType>>();
            },
            D(diameters))
        .def_property(
            "type",
            [](morphio::mut::Soma* soma) { return soma->type(); },
            [](morphio::mut::Soma* soma, morphio::SomaType type) { soma->type() = type; },
            D(type))
        .def_property_readonly("surface", &morphio::mut::Soma::surface, D(surface))
        .def_property_readonly("max_distance", &morphio::mut::Soma::maxDistance, D(maxDistance))
        .def_property_readonly(
            "center",
            [](morphio::mut::Soma* soma) { return py::array(3, soma->center().data()); },
            D(center));
#undef D
}

void bind_mut_endoplasmic_reticulum(py::module& m) {
    py::class_<morphio::mut::EndoplasmicReticulum>(m,
                                                   "EndoplasmicReticulum",
                                                   DOC(morphio, mut, EndoplasmicReticulum))
        .def(py::init<>())
        .def(py::init<const std::vector<uint32_t>&,
                      const std::vector<morphio::floatType>&,
                      const std::vector<morphio::floatType>&,
                      const std::vector<uint32_t>&>())
        .def(py::init<const morphio::EndoplasmicReticulum&>())
        .def(py::init<const morphio::mut::EndoplasmicReticulum&>())

        .def_property(
            "section_indices",
            [](morphio::mut::EndoplasmicReticulum* reticulum) {
                return py::array(static_cast<py::ssize_t>(reticulum->sectionIndices().size()),
                                 reticulum->sectionIndices().data());
            },
            [](morphio::mut::EndoplasmicReticulum* reticulum, py::array_t<uint32_t> indices) {
                reticulum->sectionIndices() = indices.cast<std::vector<uint32_t>>();
            },
            DOC(morphio, mut, EndoplasmicReticulum, sectionIndices))
        .def_property(
            "volumes",
            [](morphio::mut::EndoplasmicReticulum* reticulum) {
                return py::array(static_cast<py::ssize_t>(reticulum->volumes().size()),
                                 reticulum->volumes().data());
            },
            [](morphio::mut::EndoplasmicReticulum* reticulum,
               py::array_t<morphio::floatType> volumes) {
                reticulum->volumes() = volumes.cast<std::vector<morphio::floatType>>();
            },
            DOC(morphio, mut, EndoplasmicReticulum, volumes))
        .def_property(
            "surface_areas",
            [](morphio::mut::EndoplasmicReticulum* reticulum) {
                return py::array(static_cast<py::ssize_t>(reticulum->surfaceAreas().size()),
                                 reticulum->surfaceAreas().data());
            },
            [](morphio::mut::EndoplasmicReticulum* reticulum,
               py::array_t<morphio::floatType> areas) {
                reticulum->surfaceAreas() = areas.cast<std::vector<morphio::floatType>>();
            },
            DOC(morphio, mut, EndoplasmicReticulum, surfaceAreas))
        .def_property(
            "filament_counts",
            [](morphio::mut::EndoplasmicReticulum* reticulum) {
                return py::array(static_cast<py::ssize_t>(reticulum->filamentCounts().size()),
                                 reticulum->filamentCounts().data());
            },
            [](morphio::mut::EndoplasmicReticulum* reticulum, py::array_t<uint32_t> counts) {
                reticulum->filamentCounts() = counts.cast<std::vector<uint32_t>>();
            },
            DOC(morphio, mut, EndoplasmicReticulum, filamentCounts));
}

void bind_mut_dendritic_spine(py::module& m) {
    py::class_<morphio::mut::DendriticSpine, morphio::mut::Morphology>(
        m, "DendriticSpine", DOC(morphio, mut, DendriticSpine))
        .def(py::init<>())
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::mut::DendriticSpine>(py::str(arg));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__")
        .def_property_readonly("sections",
                               &morphio::mut::DendriticSpine::sections,
                               DOC(morphio, mut, Morphology, sections))
        .def_property_readonly("root_sections",
                               &morphio::mut::DendriticSpine::rootSections,
                               DOC(morphio, mut, Morphology, rootSections),
                               py::return_value_policy::reference)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::DendriticSpine::*)(
                 const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &morphio::mut::Morphology::appendRootSection),
             DOC(morphio, mut, Morphology, appendRootSection),
             "point_level_properties"_a,
             "section_type"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::DendriticSpine::*)(
                 const morphio::Section&, bool)>(&morphio::mut::Morphology::appendRootSection),
             DOC(morphio, mut, Morphology, appendRootSection_2),
             "immutable_section"_a,
             "recursive"_a = false)

        .def_property(
            "post_synaptic_density",
            [](const morphio::mut::DendriticSpine& dendritic_spine) {
                return dendritic_spine.postSynapticDensity();
            },
            [](morphio::mut::DendriticSpine* dendritic_spine,
               const std::vector<morphio::Property::DendriticSpine::PostSynapticDensity>& psds) {
                dendritic_spine->postSynapticDensity() = psds;
            },
            DOC(morphio, mut, DendriticSpine, postSynapticDensity))
        .def_property_readonly("cell_family",
                               &morphio::mut::DendriticSpine::cellFamily,
                               DOC(morphio, enums, CellFamily))
        .def(
            "write",
            [](morphio::mut::DendriticSpine* morph, py::object arg) { morph->write(py::str(arg)); },
            "filename"_a);
}
