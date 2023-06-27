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

#include <array>
#include <memory>  // std::make_unique

#include "bind_enums.h"
#include "bindings_utils.h"

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
        .def_property_readonly("sections",
                               &Morphology::sections,
                               "Returns a list containing IDs of all sections. "
                               "The first section of the vector is the soma section")
        .def_property_readonly("root_sections",
                               &Morphology::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)",
                               py::return_value_policy::reference)
        .def_property_readonly("soma",
                               static_cast<std::shared_ptr<morphio::mut::Soma>& (Morphology::*) ()>(
                                   &Morphology::soma),
                               "Returns a reference to the soma object\n\n"
                               "Note: multiple morphologies can share the same Soma "
                               "instance")
        .def_property_readonly("mitochondria",
                               static_cast<morphio::mut::Mitochondria& (Morphology::*) ()>(
                                   &Morphology::mitochondria),
                               "Returns a reference to the mitochondria container class")
        .def_property_readonly("endoplasmic_reticulum",
                               static_cast<morphio::mut::EndoplasmicReticulum& (Morphology::*) ()>(
                                   &Morphology::endoplasmicReticulum),
                               "Returns a reference to the endoplasmic reticulum container class")
        .def_property_readonly("annotations",
                               &Morphology::annotations,
                               "Returns a list of annotations")
        .def_property_readonly("markers",
                               &Morphology::markers,
                               "Returns the list of NeuroLucida markers")
        .def("section",
             &Morphology::section,
             "Returns the section with the given id\n\n"
             "Note: multiple morphologies can share the same Section "
             "instances",
             "section_id"_a)
        .def("build_read_only",
             &Morphology::buildReadOnly,
             "Returns the data structure used to create read-only "
             "morphologies")
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 Morphology::*)(const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &Morphology::appendRootSection),
             "Append a root Section\n",
             "point_level_properties"_a,
             "section_type"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 Morphology::*)(const morphio::Section&, bool)>(&Morphology::appendRootSection),
             "Append the existing immutable Section as a root section\n"
             "If recursive == true, all descendent will be appended as "
             "well",
             "immutable_section"_a,
             "recursive"_a = false)

        .def("delete_section",
             &Morphology::deleteSection,
             "Delete the given section\n"
             "\n"
             "Will silently fail if the section is not part of the tree\n"
             "\n"
             "If recursive == true, all descendent sections will be "
             "deleted as well\n"
             "Else, children will be re-attached to their grand-parent",
             "section"_a,
             "recursive"_a = true)
        .def("as_immutable", [](const Morphology* morph) { return morphio::Morphology(*morph); })
        .def_property_readonly("connectivity",
                               &Morphology::connectivity,
                               "Return the graph connectivity of the morphology "
                               "where each section is seen as a node\nNote: -1 is the soma node")
        .def_property_readonly("cell_family",
                               &Morphology::cellFamily,
                               "Returns the cell family (neuron or glia)")
        .def_property_readonly("soma_type", &Morphology::somaType, "Returns the soma type")
        .def_property_readonly("version", &Morphology::version, "Returns the version")
        .def("remove_unifurcations",
             static_cast<void (Morphology::*)()>(&Morphology::removeUnifurcations),
             "Fixes the morphology single child sections and issues warnings"
             "if the section starts and ends are inconsistent")
        .def(
            "write",
            [](Morphology* morph, py::object arg) { morph->write(py::str(arg)); },
            "Write file to H5, SWC, ASC format depending on filename "
            "extension",
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
            py::keep_alive<0, 1>() /* Essential: keep object alive
                                      while iterator exists */
            ,
            "Section iterator that runs successively on every "
            "neurite\n"
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
             "Append the existing mutable Section as a root section\n"
             "If recursive == true, all descendent will be appended as well",
             "mutable_section"_a,
             "recursive"_a = false);
}

void bind_mut_glialcell(py::module& m) {
    py::class_<morphio::mut::GlialCell, morphio::mut::Morphology>(
        m, "GlialCell", "Class representing a mutable Glial Cell")
        .def(py::init<>())
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::mut::GlialCell>(py::str(arg));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__");
}

void bind_mut_mitochondria(py::module& m) {
    using morphio::mut::Mitochondria;
    using morphio::mut::MitoSection;
    py::class_<Mitochondria>(m, "Mitochondria", "Class representing a mutable Mitochondria")
        .def(py::init<>())
        .def_property_readonly("root_sections",
                               &Mitochondria::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)",
                               py::return_value_policy::reference)
        .def_property_readonly("sections",
                               &Mitochondria::sections,
                               "Return a dict where key is the mitochondrial section ID"
                               " and value is the mithochondrial section")
        .def("is_root",
             &Mitochondria::isRoot,
             "Return True if section is a root section",
             "section_id"_a)
        .def("parent",
             &Mitochondria::parent,
             "Returns the parent mithochondrial section ID",
             "section_id"_a)
        .def("children", &Mitochondria::children, "section_id"_a)
        .def("section",
             &Mitochondria::section,
             "Get a reference to the given mithochondrial section\n\n"
             "Note: multiple mitochondria can shared the same references",
             "section_id"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<MitoSection> (Mitochondria::*)(
                 const morphio::Property::MitochondriaPointLevel&)>(
                 &Mitochondria::appendRootSection),
             "Append a new root MitoSection",
             "point_level_properties"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<MitoSection> (Mitochondria::*)(const morphio::MitoSection&,
                                                                        bool recursive)>(
                 &Mitochondria::appendRootSection),
             "Append a new root MitoSection (if recursive == true, all "
             "descendent will be appended "
             "as well)",
             "immutable_section"_a,
             "recursive"_a = true)
        .def("append_root_section",
             static_cast<std::shared_ptr<MitoSection> (
                 Mitochondria::*)(const std::shared_ptr<MitoSection>&, bool recursive)>(
                 &Mitochondria::appendRootSection),
             "Append a new root MitoSection (if recursive == true, all "
             "descendent will be appended "
             "as well)",
             "section"_a,
             "recursive"_a = true)
        .def(
            "depth_begin",
            [](Mitochondria* morph, std::shared_ptr<MitoSection> section) {
                return py::make_iterator(morph->depth_begin(section), morph->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed "
            "starting\n"
            "at each root section",
            "section_id"_a = -1)
        .def(
            "breadth_begin",
            [](Mitochondria* morph, std::shared_ptr<MitoSection> section) {
                return py::make_iterator(morph->breadth_begin(section), morph->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Breadth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed "
            "starting\n"
            "at each root section",
            "section_id"_a = -1)
        .def(
            "upstream_begin",
            [](Mitochondria* morph, std::shared_ptr<MitoSection> section) {
                return py::make_iterator(morph->upstream_begin(section), morph->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Upstream iterator starting at a given section id\n\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a = -1);
}

void bind_mut_mitosection(py::module& m) {
    using morphio::mut::MitoSection;
    using mitosection_floats_f = std::vector<morphio::floatType>& (MitoSection::*) ();
    using mitosection_ints_f = std::vector<uint32_t>& (MitoSection::*) ();

    py::class_<MitoSection, std::shared_ptr<MitoSection>>(
        m, "MitoSection", "Class representing a mutable Mitochondrial Section")
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
        .def("has_same_shape", &MitoSection::hasSameShape)
        .def("append_section",
             static_cast<std::shared_ptr<MitoSection> (MitoSection::*)(
                 const morphio::Property::MitochondriaPointLevel&)>(&MitoSection::appendSection),
             "Append a new MitoSection to this mito section",
             "point_level_properties"_a)
        .def("append_section",
             static_cast<std::shared_ptr<MitoSection> (MitoSection::*)(
                 const std::shared_ptr<MitoSection>&, bool)>(&MitoSection::appendSection),
             "Append a copy of the section to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "section"_a,
             "recursive"_a = false)
        .def("append_section",
             static_cast<std::shared_ptr<MitoSection> (
                 MitoSection::*)(const morphio::MitoSection&, bool)>(&MitoSection::appendSection),
             "Append the existing immutable MitoSection to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "immutable_section"_a,
             "recursive"_a = false);
}

void bind_mut_section(py::module& m) {
    py::class_<morphio::mut::Section, std::shared_ptr<morphio::mut::Section>>(
        m, "Section", "Class representing a mutable Section")
        .def("__str__",
             [](const morphio::mut::Section& section) {
                 std::stringstream ss;
                 ss << section;
                 return ss.str();
             })
        .def_property_readonly("id", &morphio::mut::Section::id, "Return the section ID")
        .def_property(
            "type",
            static_cast<const morphio::SectionType& (morphio::mut::Section::*) () const>(
                &morphio::mut::Section::type),
            [](morphio::mut::Section* section, morphio::SectionType _type) {
                section->type() = _type;
            },
            "Returns the morphological type of this section "
            "(dendrite, axon, ...)")
        .def_property(
            "points",
            [](morphio::mut::Section* section) {
                return py::array(static_cast<py::ssize_t>(section->points().size()),
                                 section->points().data());
            },
            [](morphio::mut::Section* section, py::array_t<morphio::floatType> _points) {
                section->points() = array_to_points(_points);
            },
            "Returns the coordinates (x,y,z) of all points of this section")
        .def_property(
            "diameters",
            [](morphio::mut::Section* section) {
                return py::array(static_cast<py::ssize_t>(section->diameters().size()),
                                 section->diameters().data());
            },
            [](morphio::mut::Section* section, py::array_t<morphio::floatType> _diameters) {
                section->diameters() = _diameters.cast<std::vector<morphio::floatType>>();
            },
            "Returns the diameters of all points of this section")
        .def_property(
            "perimeters",
            [](morphio::mut::Section* section) {
                return py::array(static_cast<py::ssize_t>(section->perimeters().size()),
                                 section->perimeters().data());
            },
            [](morphio::mut::Section* section, py::array_t<morphio::floatType> _perimeters) {
                section->perimeters() = _perimeters.cast<std::vector<morphio::floatType>>();
            },
            "Returns the perimeters of all points of this section")
        .def_property_readonly("is_root",
                               &morphio::mut::Section::isRoot,
                               "Return True if section is a root section")
        .def_property_readonly("parent",
                               &morphio::mut::Section::parent,
                               "Get the parent ID\n\n"
                               "Note: Root sections return -1")
        .def_property_readonly("children",
                               &morphio::mut::Section::children,
                               "Returns a list of children IDs")
        .def("is_heterogeneous",
             &morphio::mut::Section::isHeterogeneous,
             "Returns true if the tree downtream (downstream = true) or upstream (downstream = "
             "false)\n"
             "has the same type as the current section.",
             py::arg("downstream") = true)
        .def("has_same_shape", &morphio::mut::Section::hasSameShape)

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
                 const morphio::Section&, bool)>(&morphio::mut::Section::appendSection),
             "Append the existing immutable Section to this section"
             "If recursive == true, all descendent will be appended as well",
             "immutable_section"_a,
             "recursive"_a = false)

        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (
                 morphio::mut::Section::*)(std::shared_ptr<morphio::mut::Section>, bool)>(
                 &morphio::mut::Section::appendSection),
             "Append the existing mutable Section to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "mutable_section"_a,
             "recursive"_a = false)

        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::Section::*)(
                 const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &morphio::mut::Section::appendSection),
             "Append a new Section to this section\n"
             " If section_type is omitted or set to 'undefined'"
             " the type of the parent section will be used",
             "point_level_properties"_a,
             "section_type"_a = morphio::SectionType::SECTION_UNDEFINED);
}

void bind_mut_soma(py::module& m) {
    py::class_<morphio::mut::Soma, std::shared_ptr<morphio::mut::Soma>>(
        m, "Soma", "Class representing a mutable Soma")
        .def(py::init<const morphio::Property::PointLevel&>())
        .def_property(
            "points",
            [](morphio::mut::Soma* soma) {
                return py::array(static_cast<py::ssize_t>(soma->points().size()),
                                 soma->points().data());
            },
            [](morphio::mut::Soma* soma, py::array_t<morphio::floatType> _points) {
                soma->points() = array_to_points(_points);
            },
            "Returns the coordinates (x,y,z) of all soma point")
        .def_property(
            "diameters",
            [](morphio::mut::Soma* soma) {
                return py::array(static_cast<py::ssize_t>(soma->diameters().size()),
                                 soma->diameters().data());
            },
            [](morphio::mut::Soma* soma, py::array_t<morphio::floatType> _diameters) {
                soma->diameters() = _diameters.cast<std::vector<morphio::floatType>>();
            },
            "Returns the diameters of all soma points")
        .def_property(
            "type",
            [](morphio::mut::Soma* soma) { return soma->type(); },
            [](morphio::mut::Soma* soma, morphio::SomaType type) { soma->type() = type; },
            "Returns the soma type")
        .def_property_readonly("surface",
                               &morphio::mut::Soma::surface,
                               "Returns the soma surface\n\n"
                               "Note: the soma surface computation depends on the soma type")
        .def_property_readonly("max_distance",
                               &morphio::mut::Soma::maxDistance,
                               "Return the maximum distance between the center of gravity "
                               "and any of the soma points")
        .def_property_readonly(
            "center",
            [](morphio::mut::Soma* soma) { return py::array(3, soma->center().data()); },
            "Returns the center of gravity of the soma points");
}

void bind_mut_endoplasmic_reticulum(py::module& m) {
    py::class_<morphio::mut::EndoplasmicReticulum>(
        m, "EndoplasmicReticulum", "Class representing a mutable Endoplasmic Reticulum")
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
            "Returns the list of neuronal section indices")
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
            "Returns the volumes for each neuronal section")

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
            "Returns the surface areas for each neuronal section")

        .def_property(
            "filament_counts",
            [](morphio::mut::EndoplasmicReticulum* reticulum) {
                return py::array(static_cast<py::ssize_t>(reticulum->filamentCounts().size()),
                                 reticulum->filamentCounts().data());
            },
            [](morphio::mut::EndoplasmicReticulum* reticulum, py::array_t<uint32_t> counts) {
                reticulum->filamentCounts() = counts.cast<std::vector<uint32_t>>();
            },
            "Returns the number of filaments for each neuronal section");
}

void bind_mut_dendritic_spine(py::module& m) {
    py::class_<morphio::mut::DendriticSpine, morphio::mut::Morphology>(
        m, "DendriticSpine", "Class representing a mutable Dendritic Spine")
        .def(py::init<>())
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::mut::DendriticSpine>(py::str(arg));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__")
        .def_property_readonly("sections",
                               &morphio::mut::DendriticSpine::sections,
                               "Returns a list containing IDs of all sections.")
        .def_property_readonly("root_sections",
                               &morphio::mut::DendriticSpine::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)",
                               py::return_value_policy::reference)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::DendriticSpine::*)(
                 const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &morphio::mut::Morphology::appendRootSection),
             "Append a root Section\n",
             "point_level_properties"_a,
             "section_type"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::DendriticSpine::*)(
                 const morphio::Section&, bool)>(&morphio::mut::Morphology::appendRootSection),
             "Append the existing immutable Section as a root section\n"
             "If recursive == true, all descendent will be appended as "
             "well",
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
            "Returns the post synaptic density values")
        .def_property_readonly("cell_family",
                               &morphio::mut::DendriticSpine::cellFamily,
                               "Returns the cell family")
        .def(
            "write",
            [](morphio::mut::DendriticSpine* morph, py::object arg) { morph->write(py::str(arg)); },
            "filename"_a);
}
