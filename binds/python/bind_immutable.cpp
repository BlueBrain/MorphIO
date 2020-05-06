#include "bind_immutable.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/iostream.h>  // py::add_ostream_redirect

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/enums.h>
#include <morphio/glial_cell.h>
#include <morphio/mut/morphology.h>
#include <morphio/soma.h>
#include <morphio/types.h>

#include "bind_enums.h"
#include "bindings_utils.h"

namespace py = pybind11;

void bind_immutable_module(py::module& m) {
    using namespace py::literals;

    // http://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=iostream#capturing-standard-output-from-ostream
    py::add_ostream_redirect(m, "ostream_redirect");

    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const std::string&, unsigned int>(),
             "filename"_a,
             "options"_a = morphio::enums::Option::NO_MODIFIER)
        .def(py::init<morphio::mut::Morphology&>())
        .def(py::init([](py::object arg, unsigned int options) {
                 return std::unique_ptr<morphio::Morphology>(
                     new morphio::Morphology(py::str(arg), options));
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
            "iter_type controls the order of iteration on sections of a given neurite. 2 values "
            "can be passed:\n"
            "- morphio.IterType.depth_first (default)\n"
            "- morphio.IterType.breadth_first (default)\n"
            "iter_type"_a = IterType::DEPTH_FIRST);

    py::class_<morphio::GlialCell, morphio::Morphology>(m, "GlialCell")
        .def(py::init<const std::string&>())
        .def(py::init([](py::object arg) {
                 return std::unique_ptr<morphio::GlialCell>(new morphio::GlialCell(py::str(arg)));
             }),
             "filename"_a,
             "Additional Ctor that accepts as filename any python object that implements __repr__ "
             "or __str__");

    py::class_<morphio::Mitochondria>(
        m,
        "Mitochondria",
        "The entry-point class to access mitochondrial data\n"
        "By design, it is the equivalent of the Morphology class but at the mitochondrial level\n"
        "As the Morphology class, it implements a section accessor and a root section accessor\n"
        "returning views on the Properties object for the queried mitochondrial section")
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

    py::class_<morphio::EndoplasmicReticulum>(
        m,
        "EndoplasmicReticulum",
        "The entry-point class to access endoplasmic reticulum data\n"
        "Spec "
        "https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/"
        "h5v1.html")
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


    py::class_<morphio::Soma>(m, "Soma")
        .def(py::init<const morphio::Soma&>())
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

    py::class_<morphio::Section>(m, "Section")
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
            "diameters",
            [](morphio::Section* section) { return span_to_ndarray(section->diameters()); },
            "Returns list of section's point diameters")
        .def_property_readonly(
            "perimeters",
            [](morphio::Section* section) { return span_to_ndarray(section->perimeters()); },
            "Returns list of section's point perimeters")

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
                    throw morphio::MorphioError("Only iteration types depth_first, breadth_first and "
                                              "upstream are supported");
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator\n"
            "\n"
            "iter_type controls the iteration order. 3 values can be passed:\n"
            "- morphio.IterType.depth_first (default)\n"
            "- morphio.IterType.breadth_first\n"
            "- morphio.IterType.upstream\n",
            "iter_type"_a = IterType::DEPTH_FIRST);


    py::class_<morphio::MitoSection>(m, "MitoSection")
        // Topology-related member functions
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
                    throw morphio::MorphioError("Only iteration types depth_first, breadth_first and "
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
