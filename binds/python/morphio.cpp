#include <pybind11/iostream.h>  // py::add_ostream_redirect
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/dendritic_spine.h>
#include <morphio/endoplasmic_reticulum.h>
#include <morphio/enums.h>
#include <morphio/glial_cell.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/mut/dendritic_spine.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/soma.h>
#include <morphio/soma.h>
#include <morphio/types.h>

#include <array>
#include <memory>  // std::make_unique

#include "bind_enums.h"
#include "bind_misc.h"
#include "bind_vasculature.h"
#include "bindings_utils.h"


namespace py = pybind11;

PYBIND11_MODULE(_morphio, m) {
    using namespace py::literals;

    // http://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=iostream#capturing-standard-output-from-ostream
    py::add_ostream_redirect(m, "ostream_redirect");

    bind_misc(m);

    // ############################
    // # Define immutable classes #
    // ############################
    auto Morphology_class = py::class_<morphio::Morphology>(
        m, "Morphology", "Class representing a complete morphology");
    auto GlialCell_class = py::class_<morphio::GlialCell, morphio::Morphology>(
        m, "GlialCell", "Class representing a Glial Cell");
    auto Mitochondria_class = py::class_<morphio::Mitochondria>(
        m,
        "Mitochondria",
        "The entry-point class to access mitochondrial data\n"
        "By design, it is the equivalent of the Morphology class but at the mitochondrial level\n"
        "As the Morphology class, it implements a section accessor and a root section accessor\n"
        "returning views on the Properties object for the queried mitochondrial section");
    auto EndoplasmicReticulum_class = py::class_<morphio::EndoplasmicReticulum>(
        m,
        "EndoplasmicReticulum",
        "The entry-point class to access endoplasmic reticulum data\n"
        "Spec "
        "https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/"
        "h5v1.html");
    auto Soma_class = py::class_<morphio::Soma>(m, "Soma", "Class representing a Soma");
    auto Section_class = py::class_<morphio::Section>(m, "Section", "Class representing a Section");
    auto MitoSection_class = py::class_<morphio::MitoSection>(
        m, "MitoSection", "Class representing a Mitochondrial Section");
    auto DendriticSpine_class = py::class_<morphio::DendriticSpine, morphio::Morphology>(
        m, "DendriticSpine", "Class representing a Dendritic Spine");

    // ###########################################
    // # Define 'mut' module for mutable classes #
    // ###########################################
    py::module mut_module = m.def_submodule("mut");

    // ##########################
    // # Define mutable classes #
    // ##########################
    auto Morphology_mut_class = py::class_<morphio::mut::Morphology>(
        mut_module, "Morphology", "Class representing a mutable Morphology");
    auto GlialCell_mut_class = py::class_<morphio::mut::GlialCell, morphio::mut::Morphology>(
        mut_module, "GlialCell", "Class representing a mutable Glial Cell");
    auto Mitochondria_mut_class = py::class_<morphio::mut::Mitochondria>(
        mut_module, "Mitochondria", "Class representing a mutable Mitochondria");
    auto MitoSection_mut_class =
        py::class_<morphio::mut::MitoSection, std::shared_ptr<morphio::mut::MitoSection>>(
            mut_module, "MitoSection", "Class representing a mutable Mitochondrial Section");
    auto Section_mut_class =
        py::class_<morphio::mut::Section, std::shared_ptr<morphio::mut::Section>>(
            mut_module, "Section", "Class representing a mutable Section");
    auto Soma_mut_class = py::class_<morphio::mut::Soma, std::shared_ptr<morphio::mut::Soma>>(
        mut_module, "Soma", "Class representing a mutable Soma");
    auto EndoplasmicReticulum_mut_class = py::class_<morphio::mut::EndoplasmicReticulum>(
        mut_module, "EndoplasmicReticulum", "Class representing a mutable Endoplasmic Reticulum");
    auto DendriticSpine_mut_class =
        py::class_<morphio::mut::DendriticSpine, morphio::mut::Morphology>(
            mut_module, "DendriticSpine", "Class representing a mutable Dendritic Spine");

    // ######################################
    // Define methods for immutable classes #
    // ######################################
    Morphology_class
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
        .def_property_readonly("soma", &morphio::Morphology::soma, "Returns the soma object")
        .def_property_readonly("mitochondria",
                               &morphio::Morphology::mitochondria,
                               "Returns the soma object")
        .def_property_readonly("annotations",
                               &morphio::Morphology::annotations,
                               "Returns a list of annotations")
        .def_property_readonly("markers",
                               &morphio::Morphology::markers,
                               "Returns the list of NeuroLucida markers")
        .def_property_readonly("endoplasmic_reticulum",
                               &morphio::Morphology::endoplasmicReticulum,
                               "Returns the endoplasmic reticulum object")
        .def_property_readonly("root_sections",
                               &morphio::Morphology::rootSections,
                               "Returns a list of all root sections "
                               "(sections whose parent ID are -1)")
        .def_property_readonly("sections",
                               &morphio::Morphology::sections,
                               "Returns a vector containing all sections objects\n\n"
                               "Notes:\n"
                               "- Soma is not included\n"
                               "- First section ID is 1 (0 is reserved for the soma)\n"
                               "- To select sections by ID use: Morphology::section(id)")

        .def("section",
             &morphio::Morphology::section,
             "Returns the Section with the given id\n"
             "throw RawDataError if the id is out of range",
             "section_id"_a)

        // Property accessors
        .def_property_readonly(
            "points",
            [](morphio::Morphology* morpho) {
                const auto& data = morpho->points();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a list with all points from all sections (soma points are not included)\n"
            "Note: points belonging to the n'th section are located at indices:\n"
            "[Morphology.sectionOffsets(n), Morphology.sectionOffsets(n+1)[")

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
            "Returns a list with all diameters from all sections (soma points are not included)\n"
            "Note: diameters belonging to the n'th section are located at indices:\n"
            "[Morphology.sectionOffsets(n), Morphology.sectionOffsets(n+1)[")
        .def_property_readonly(
            "perimeters",
            [](const morphio::Morphology& obj) {
                const auto& data = obj.perimeters();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a list with all perimeters from all sections (soma points are not included)\n"
            "Note: perimeters belonging to the n'th section are located at indices:\n"
            "[Morphology.sectionOffsets(n), Morphology.sectionOffsets(n+1)[")
        .def_property_readonly(
            "section_offsets",
            [](const morphio::Morphology& morpho) { return as_pyarray(morpho.sectionOffsets()); },
            "Returns a list with N+1 offsets to access data of a specific section in the points\n"
            "and diameters arrays (size N).\n"
            "\n"
            "Example: accessing diameters of n'th section will be located in the diameters\n"
            "array: diameters[section_offsets[n]: section_offsets[n + 1]]\n"
            "\n"
            "Note: for convenience, the last element of this array is equal to the length of the\n"
            "points/diameters arrays so that the above example works also for the last section.")
        .def_property_readonly(
            "section_types",
            [](const morphio::Morphology& morph) {
                const auto& data = morph.sectionTypes();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a vector with the section type of every section")
        .def_property_readonly("connectivity",
                               &morphio::Morphology::connectivity,
                               "Return the graph connectivity of the morphology "
                               "where each section is seen as a node\nNote: -1 is the soma node")
        .def_property_readonly("soma_type", &morphio::Morphology::somaType, "Returns the soma type")
        .def_property_readonly("cell_family",
                               &morphio::Morphology::cellFamily,
                               "Returns the cell family (neuron or glia)")
        .def_property_readonly("version", &morphio::Morphology::version, "Returns the version")

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

    GlialCell_class.def(py::init<const std::string&>())
        .def(py::init(
                 [](py::object arg) { return std::make_unique<morphio::GlialCell>(py::str(arg)); }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python object that implements __repr__ "
             "or __str__");

    Mitochondria_class
        .def("section",
             &morphio::Mitochondria::section,
             "Returns the mithochondrial section with the given ID",
             "section_id"_a)
        .def_property_readonly("sections",
                               &morphio::Mitochondria::sections,
                               "Returns a list of all mitochondrial sections")
        .def_property_readonly(
            "root_sections",
            &morphio::Mitochondria::rootSections,
            "Returns a list of all root sections (section whose parent ID is -1)");

    EndoplasmicReticulum_class
        .def_property_readonly("section_indices",
                               &morphio::EndoplasmicReticulum::sectionIndices,
                               "Returns the list of neuronal section indices")
        .def_property_readonly("volumes",
                               &morphio::EndoplasmicReticulum::volumes,
                               "Returns the list of neuronal section indices")
        .def_property_readonly("surface_areas",
                               &morphio::EndoplasmicReticulum::surfaceAreas,
                               "Returns the surface areas for each neuronal section")
        .def_property_readonly("filament_counts",
                               &morphio::EndoplasmicReticulum::filamentCounts,
                               "Returns the number of filaments for each neuronal section");

    Soma_class.def(py::init<const morphio::Soma&>())
        .def_property_readonly(
            "points",
            [](morphio::Soma* soma) { return span_array_to_ndarray(soma->points()); },
            "Returns the coordinates (x,y,z) of all soma point")
        .def_property_readonly(
            "diameters",
            [](morphio::Soma* soma) { return span_to_ndarray(soma->diameters()); },
            "Returns the diameters of all soma points")

        .def_property_readonly(
            "center",
            [](morphio::Soma* soma) { return py::array(3, soma->center().data()); },
            "Returns the center of gravity of the soma points")
        .def_property_readonly("max_distance",
                               &morphio::Soma::maxDistance,
                               "Return the maximum distance between the center of gravity "
                               "and any of the soma points")
        .def_property_readonly("type", &morphio::Soma::type, "Returns the soma type")

        .def_property_readonly("surface",
                               &morphio::Soma::surface,
                               "Returns the soma surface\n\n"
                               "Note: the soma surface computation depends on the soma type");

    Section_class
        .def("__str__",
             [](const morphio::Section& section) {
                 std::stringstream ss;
                 ss << section;
                 return ss.str();
             })

        // Topology-related member functions
        .def_property_readonly("parent",
                               &morphio::Section::parent,
                               "Returns the parent section of this section\n"
                               "throw MissingParentError is the section doesn't have a parent")
        .def_property_readonly("is_root",
                               &morphio::Section::isRoot,
                               "Returns true if this section is a root section (parent ID == -1)")
        .def_property_readonly("children",
                               &morphio::Section::children,
                               "Returns a list of children sections")

        // Property-related accessors
        .def_property_readonly(
            "id",
            &morphio::Section::id,
            "Returns the section ID\n"
            "The section ID can be used to query sections via Morphology::section(uint32_t id)")
        .def_property_readonly("type",
                               &morphio::Section::type,
                               "Returns the morphological type of this section "
                               "(dendrite, axon, ...)")
        .def_property_readonly(
            "points",
            [](morphio::Section* section) { return span_array_to_ndarray(section->points()); },
            "Returns list of section's point coordinates")

        .def_property_readonly(
            "n_points",
            [](const morphio::Section& section) { return section.points().size(); },
            "Returns the number of points in section")

        .def_property_readonly(
            "diameters",
            [](morphio::Section* section) { return span_to_ndarray(section->diameters()); },
            "Returns list of section's point diameters")
        .def_property_readonly(
            "perimeters",
            [](morphio::Section* section) { return span_to_ndarray(section->perimeters()); },
            "Returns list of section's point perimeters")

        .def("is_heterogeneous",
             &morphio::Section::isHeterogeneous,
             "Returns true if the tree downtream (downstream = true) or upstream (downstream = "
             "false)\n"
             "has the same type as the current section.",
             py::arg("downstream") = true)

        .def("has_same_shape", &morphio::Section::hasSameShape)

        // Iterators
        .def(
            "iter",
            [](morphio::Section* section, IterType type) {
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


    // Topology-related member functions
    MitoSection_class
        .def_property_readonly("parent",
                               &morphio::MitoSection::parent,
                               "Returns the parent mitochondrial section of this section\n"
                               "throw MissingParentError is the section doesn't have a parent")
        .def_property_readonly("is_root",
                               &morphio::MitoSection::isRoot,
                               "Returns true if this section is a root section (parent ID == -1)")
        .def_property_readonly("children",
                               &morphio::MitoSection::children,
                               "Returns a list of children mitochondrial sections")

        // Property-related accesors
        .def_property_readonly(
            "id",
            &morphio::MitoSection::id,
            "Returns the section ID\n"
            "The section ID can be used to query sections via Mitochondria::section(uint32_t id)")
        .def_property_readonly(
            "neurite_section_ids",
            [](morphio::MitoSection* section) {
                return span_to_ndarray(section->neuriteSectionIds());
            },
            "Returns list of neuronal section IDs associated to each point "
            "of this mitochondrial section")
        .def_property_readonly(
            "diameters",
            [](morphio::MitoSection* section) { return span_to_ndarray(section->diameters()); },
            "Returns list of section's point diameters")
        .def_property_readonly(
            "relative_path_lengths",
            [](morphio::MitoSection* section) {
                return span_to_ndarray(section->relativePathLengths());
            },
            "Returns list of relative distances between the start of the "
            "neuronal section and each point of the mitochondrial section\n\n"
            "Note: - a relative distance of 0 means the mitochondrial point is at the "
            "beginning of the neuronal section\n"
            "      - a relative distance of 1 means the mitochondrial point is at the "
            "end of the neuronal section\n")

        .def("has_same_shape", &morphio::MitoSection::hasSameShape)

        // Iterators
        .def(
            "iter",
            [](morphio::MitoSection* section, IterType type) {
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

    DendriticSpine_class
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::DendriticSpine>(py::str(arg));
             }),
             "filename"_a)
        .def_property_readonly("root_sections",
                               &morphio::DendriticSpine::rootSections,
                               "Returns a list of all root sections "
                               "(sections whose parent ID are -1)")
        .def_property_readonly("sections",
                               &morphio::DendriticSpine::sections,
                               "Returns a vector containing all sections objects\n\n"
                               "Notes:\n"
                               "- Soma is not included\n"
                               "- First section ID is 1 (0 is reserved for the soma)\n"
                               "- To select sections by ID use: DendriticSpine::section(id)")

        .def("section",
             &morphio::DendriticSpine::section,
             "Returns the Section with the given id\n"
             "throw RawDataError if the id is out of range",
             "section_id"_a)

        // Property accessors
        .def_property_readonly(
            "points",
            [](morphio::DendriticSpine* morpho) {
                const auto& data = morpho->points();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a list with all points from all sections\n"
            "Note: points belonging to the n'th section are located at indices:\n"
            "[DendriticSpine.sectionOffsets(n), DendriticSpine.sectionOffsets(n+1)[")
        .def_property_readonly(
            "diameters",
            [](const morphio::DendriticSpine& morpho) {
                const auto& data = morpho.diameters();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a list with all diameters from all sections\n"
            "Note: diameters belonging to the n'th section are located at indices:\n"
            "[DendriticSpine.sectionOffsets(n), DendriticSpine.sectionOffsets(n+1)[")
        .def_property_readonly(
            "section_offsets",
            [](const morphio::DendriticSpine& morpho) {
                return as_pyarray(morpho.sectionOffsets());
            },
            "Returns a list with offsets to access data of a specific section in the points\n"
            "and diameters arrays.\n"
            "\n"
            "Example: accessing diameters of n'th section will be located in the DIAMETERS\n"
            "array from DIAMETERS[sectionOffsets(n)] to DIAMETERS[sectionOffsets(n+1)-1]\n"
            "\n"
            "Note: for convenience, the last point of this array is the points array size\n"
            "so that the above example works also for the last section.")
        .def_property_readonly(
            "section_types",
            [](const morphio::DendriticSpine& morph) {
                const auto& data = morph.sectionTypes();
                return py::array(static_cast<py::ssize_t>(data.size()), data.data());
            },
            "Returns a vector with the section type of every section")
        .def_property_readonly("connectivity",
                               &morphio::DendriticSpine::connectivity,
                               "Return the graph connectivity of the DendriticSpine "
                               "where each section is seen as a node\nNote: -1 is the soma node")
        .def_property_readonly("cell_family",
                               &morphio::DendriticSpine::cellFamily,
                               "Returns the cell family (neuron or glia)")
        .def_property_readonly("post_synaptic_density",
                               &morphio::DendriticSpine::postSynapticDensity,
                               "Returns the post synaptic density values")
        .def_property_readonly("version", &morphio::DendriticSpine::version, "Returns the version")

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

    // ######################################
    // # Define methods for mutable classes #
    // ######################################
    Morphology_mut_class.def(py::init<>())
        .def(py::init<const std::string&, unsigned int>(),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const morphio::Morphology&, unsigned int>(),
             "morphology"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const morphio::mut::Morphology&, unsigned int>(),
             "morphology"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init([](py::object arg, unsigned int options) {
                 return std::make_unique<morphio::mut::Morphology>(py::str(arg), options);
             }),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__")

        // Cell sub-part accessors
        .def_property_readonly("sections",
                               &morphio::mut::Morphology::sections,
                               "Returns a list containing IDs of all sections. "
                               "The first section of the vector is the soma section")
        .def_property_readonly("root_sections",
                               &morphio::mut::Morphology::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)",
                               py::return_value_policy::reference)
        .def_property_readonly(
            "soma",
            static_cast<std::shared_ptr<morphio::mut::Soma>& (morphio::mut::Morphology::*) ()>(
                &morphio::mut::Morphology::soma),
            "Returns a reference to the soma object\n\n"
            "Note: multiple morphologies can share the same Soma "
            "instance")
        .def_property_readonly(
            "mitochondria",
            static_cast<morphio::mut::Mitochondria& (morphio::mut::Morphology::*) ()>(
                &morphio::mut::Morphology::mitochondria),
            "Returns a reference to the mitochondria container class")
        .def_property_readonly(
            "endoplasmic_reticulum",
            static_cast<morphio::mut::EndoplasmicReticulum& (morphio::mut::Morphology::*) ()>(
                &morphio::mut::Morphology::endoplasmicReticulum),
            "Returns a reference to the endoplasmic reticulum container class")
        .def_property_readonly("annotations",
                               &morphio::mut::Morphology::annotations,
                               "Returns a list of annotations")
        .def_property_readonly("markers",
                               &morphio::mut::Morphology::markers,
                               "Returns the list of NeuroLucida markers")
        .def("section",
             &morphio::mut::Morphology::section,
             "Returns the section with the given id\n\n"
             "Note: multiple morphologies can share the same Section "
             "instances",
             "section_id"_a)
        .def("build_read_only",
             &morphio::mut::Morphology::buildReadOnly,
             "Returns the data structure used to create read-only "
             "morphologies")
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*)(
                 const morphio::Property::PointLevel&, morphio::SectionType)>(
                 &morphio::mut::Morphology::appendRootSection),
             "Append a root Section\n",
             "point_level_properties"_a,
             "section_type"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*)(
                 const morphio::Section&, bool)>(&morphio::mut::Morphology::appendRootSection),
             "Append the existing immutable Section as a root section\n"
             "If recursive == true, all descendent will be appended as "
             "well",
             "immutable_section"_a,
             "recursive"_a = false)

        .def("delete_section",
             &morphio::mut::Morphology::deleteSection,
             "Delete the given section\n"
             "\n"
             "Will silently fail if the section is not part of the tree\n"
             "\n"
             "If recursive == true, all descendent sections will be "
             "deleted as well\n"
             "Else, children will be re-attached to their grand-parent",
             "section"_a,
             "recursive"_a = true)

        .def("as_immutable",
             [](const morphio::mut::Morphology* morph) { return morphio::Morphology(*morph); })

        .def_property_readonly("connectivity",
                               &morphio::mut::Morphology::connectivity,
                               "Return the graph connectivity of the morphology "
                               "where each section is seen as a node\nNote: -1 is the soma node")

        .def_property_readonly("cell_family",
                               &morphio::mut::Morphology::cellFamily,
                               "Returns the cell family (neuron or glia)")

        .def_property_readonly("soma_type",
                               &morphio::mut::Morphology::somaType,
                               "Returns the soma type")

        .def_property_readonly("version", &morphio::mut::Morphology::version, "Returns the version")

        .def("remove_unifurcations",
             static_cast<void (morphio::mut::Morphology::*)()>(
                 &morphio::mut::Morphology::removeUnifurcations),
             "Fixes the morphology single child sections and issues warnings"
             "if the section starts and ends are inconsistent")

        .def(
            "write",
            [](morphio::mut::Morphology* morph, py::object arg) { morph->write(py::str(arg)); },
            "Write file to H5, SWC, ASC format depending on filename "
            "extension",
            "filename"_a)

        // Iterators
        .def(
            "iter",
            [](morphio::mut::Morphology* morph, IterType type) {
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
                 morphio::mut::Morphology::*)(const std::shared_ptr<morphio::mut::Section>&, bool)>(
                 &morphio::mut::Morphology::appendRootSection),
             "Append the existing mutable Section as a root section\n"
             "If recursive == true, all descendent will be appended as well",
             "mutable_section"_a,
             "recursive"_a = false);

    GlialCell_mut_class.def(py::init<>())
        .def(py::init([](py::object arg) {
                 return std::make_unique<morphio::mut::GlialCell>(py::str(arg));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python "
             "object that implements __repr__ or __str__");


    Mitochondria_mut_class.def(py::init<>())
        .def_property_readonly("root_sections",
                               &morphio::mut::Mitochondria::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)",
                               py::return_value_policy::reference)
        .def_property_readonly("sections",
                               &morphio::mut::Mitochondria::sections,
                               "Return a dict where key is the mitochondrial section ID"
                               " and value is the mithochondrial section")
        .def("is_root",
             &morphio::mut::Mitochondria::isRoot,
             "Return True if section is a root section",
             "section_id"_a)
        .def("parent",
             &morphio::mut::Mitochondria::parent,
             "Returns the parent mithochondrial section ID",
             "section_id"_a)
        .def("children", &morphio::mut::Mitochondria::children, "section_id"_a)
        .def("section",
             &morphio::mut::Mitochondria::section,
             "Get a reference to the given mithochondrial section\n\n"
             "Note: multiple mitochondria can shared the same references",
             "section_id"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::Mitochondria::*)(
                 const morphio::Property::MitochondriaPointLevel&)>(
                 &morphio::mut::Mitochondria::appendRootSection),
             "Append a new root MitoSection",
             "point_level_properties"_a)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::MitoSection> (
                 morphio::mut::Mitochondria::*)(const morphio::MitoSection&, bool recursive)>(
                 &morphio::mut::Mitochondria::appendRootSection),
             "Append a new root MitoSection (if recursive == true, all "
             "descendent will be appended "
             "as well)",
             "immutable_section"_a,
             "recursive"_a = true)
        .def("append_root_section",
             static_cast<std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::Mitochondria::*)(
                 const std::shared_ptr<morphio::mut::MitoSection>&, bool recursive)>(
                 &morphio::mut::Mitochondria::appendRootSection),
             "Append a new root MitoSection (if recursive == true, all "
             "descendent will be appended "
             "as well)",
             "section"_a,
             "recursive"_a = true)

        .def(
            "depth_begin",
            [](morphio::mut::Mitochondria* morph,
               std::shared_ptr<morphio::mut::MitoSection> section) {
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
            [](morphio::mut::Mitochondria* morph,
               std::shared_ptr<morphio::mut::MitoSection> section) {
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
            [](morphio::mut::Mitochondria* morph,
               std::shared_ptr<morphio::mut::MitoSection> section) {
                return py::make_iterator(morph->upstream_begin(section), morph->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Upstream iterator starting at a given section id\n\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a = -1);

    using mitosection_floats_f = std::vector<morphio::floatType>& (morphio::mut::MitoSection::*) ();
    using mitosection_ints_f = std::vector<uint32_t>& (morphio::mut::MitoSection::*) ();

    MitoSection_mut_class
        .def_property_readonly("id", &morphio::mut::MitoSection::id, "Return the section ID")
        .def_property(
            "diameters",
            static_cast<mitosection_floats_f>(&morphio::mut::MitoSection::diameters),
            [](morphio::mut::MitoSection* section,
               const std::vector<morphio::floatType>& _diameters) {
                section->diameters() = _diameters;
            },
            "Returns the diameters of all points of this section")
        .def_property(
            "relative_path_lengths",
            static_cast<mitosection_floats_f>(&morphio::mut::MitoSection::pathLengths),
            [](morphio::mut::MitoSection* section,
               const std::vector<morphio::floatType>& _pathLengths) {
                section->pathLengths() = _pathLengths;
            },
            "Returns the relative distance (between 0 and 1)\n"
            "between the start of the neuronal section and each point\n"
            "of this mitochondrial section")
        .def_property(
            "neurite_section_ids",
            static_cast<mitosection_ints_f>(&morphio::mut::MitoSection::neuriteSectionIds),
            [](morphio::mut::MitoSection* section,
               const std::vector<uint32_t>& _neuriteSectionIds) {
                section->neuriteSectionIds() = _neuriteSectionIds;
            },
            "Returns the neurite section Ids of all points of this section")

        .def("has_same_shape", &morphio::mut::MitoSection::hasSameShape)

        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::MitoSection::*)(
                 const morphio::Property::MitochondriaPointLevel&)>(
                 &morphio::mut::MitoSection::appendSection),
             "Append a new MitoSection to this mito section",
             "point_level_properties"_a)

        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::MitoSection::*)(
                 const std::shared_ptr<morphio::mut::MitoSection>&, bool)>(
                 &morphio::mut::MitoSection::appendSection),
             "Append a copy of the section to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "section"_a,
             "recursive"_a = false)

        .def("append_section",
             static_cast<std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::MitoSection::*)(
                 const morphio::MitoSection&, bool)>(&morphio::mut::MitoSection::appendSection),
             "Append the existing immutable MitoSection to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "immutable_section"_a,
             "recursive"_a = false);


    Section_mut_class
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

    Soma_mut_class.def(py::init<const morphio::Property::PointLevel&>())
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
        .def_property_readonly("type", &morphio::mut::Soma::type, "Returns the soma type")
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

    EndoplasmicReticulum_mut_class.def(py::init<>())
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

    DendriticSpine_mut_class.def(py::init<>())
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

    // ##########################################
    // # Define vasculature classes and methods #
    // ##########################################
    py::module vasc_module = m.def_submodule("vasculature");
    bind_vasculature(vasc_module);
}
