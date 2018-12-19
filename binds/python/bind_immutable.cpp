#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <morphio/types.h>
#include <morphio/enums.h>
#include <morphio/mut/morphology.h>

namespace py = pybind11;
using namespace py::literals;

void bind_immutable_module(py::module &m) {

  // http://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=iostream#capturing-standard-output-from-ostream
    py::add_ostream_redirect(m, "ostream_redirect");

    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const morphio::URI&, unsigned int>(),
             "filename"_a, "options"_a=morphio::enums::Option::NO_MODIFIER)
        .def(py::init<morphio::mut::Morphology&>())
        .def("__eq__", [](const morphio::Morphology& a, const morphio::Morphology& b) {
                return a.operator==(b);
            }, py::is_operator(),
            "Are considered equal, 2 morphologies with the same:\n"
            "- point vector\n"
            "- diameter vector\n"
            "- perimeter vector\n"
            "- cell family\n"
            "- section types\n"
            "- topology (children/parent relationship)\n\n"
            "Note: the soma types are NOT required to be equal")
        .def("__ne__", [](const morphio::Morphology& a, const morphio::Morphology& b) {
                return a.operator!=(b);
            }, py::is_operator())
        .def(py::self != py::self)

        .def("as_mutable", [](const morphio::Morphology* morph) { return morphio::mut::Morphology(*morph); })


        // Cell sub-parts accessors
        .def_property_readonly("soma", &morphio::Morphology::soma,
                               "Returns the soma object")
        .def_property_readonly("mitochondria", &morphio::Morphology::mitochondria,
                               "Returns the soma object")
        .def_property_readonly("annotations", &morphio::Morphology::annotations,
                               "Returns a list of annotations")
        .def_property_readonly("root_sections", &morphio::Morphology::rootSections,
                               "Returns a list of all root sections "
                               "(sections whose parent ID are -1)")
        .def_property_readonly("sections", &morphio::Morphology::sections,
                               "Returns a vector containing all sections objects\n\n"
                               "Notes:\n"
            "- Soma is not included\n"
            "- First section ID is 1 (0 is reserved for the soma)\n"
            "- To select sections by ID use: Morphology::section(id)")

        .def("section", &morphio::Morphology::section,
             "Returns the Section with the given id\n"
             "throw RawDataError if the id is out of range",
             "section_id"_a)

        // Property accessors
        .def_property_readonly("points", [](morphio::Morphology* morpho){
                return py::array(morpho->points().size(), morpho->points().data());
            },
            "Returns a list with all points from all sections")
        .def_property_readonly("diameters", [](morphio::Morphology* morpho){
                auto diameters = morpho->diameters();
                return py::array(diameters.size(), diameters.data());
            },
            "Returns a list with all diameters from all sections (soma points are not included)")
        .def_property_readonly("perimeters", [](morphio::Morphology* obj){
                auto data = obj->perimeters();
                return py::array(data.size(), data.data());
            },
            "Returns a list with all perimeters from all sections")
        .def_property_readonly("section_types", [](morphio::Morphology* obj){
                auto data = obj->sectionTypes();
                return py::array(data.size(), data.data());
            },
            "Returns a vector with the section type of every section")
        .def_property_readonly("soma_type", &morphio::Morphology::somaType,
                               "Returns the soma type")
        .def_property_readonly("cell_family", &morphio::Morphology::cellFamily,
                               "Returns the cell family (neuron or glia)")
        .def_property_readonly("version", &morphio::Morphology::version,
                               "Returns the version")

        // Iterators
        .def("iter", [](morphio::Morphology* morpho, morphio::IterType type) {
                switch (type) {
                case morphio::IterType::DEPTH_FIRST:
                    return py::make_iterator(morpho->depth_begin(), morpho->depth_end());
                case morphio::IterType::BREADTH_FIRST:
                    return py::make_iterator(morpho->breadth_begin(), morpho->breadth_end());
                default:
                    LBTHROW(morphio::MorphioError("Only iteration types depth_first and breadth_first are supported"));
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator that runs successively on every neurite\n"
            "iter_type controls the order of iteration on sections of a given neurite. 2 values can be passed:\n"
            "- morphio.IterType.depth_first (default)\n"
            "- morphio.IterType.breadth_first (default)\n"
            "iter_type"_a=morphio::IterType::DEPTH_FIRST);



    py::class_<morphio::Mitochondria>(
        m, "Mitochondria",
        "The entry-point class to access mitochondrial data\n"
        "By design, it is the equivalent of the Morphology class but at the mitochondrial level\n"
        "As the Morphology class, it implements a section accessor and a root section accessor\n"
        "returning views on the Properties object for the queried mitochondrial section")
        .def("section", &morphio::Mitochondria::section,
             "Returns the mithochondrial section with the given ID",
             "section_id"_a)
        .def_property_readonly("sections", &morphio::Mitochondria::sections,
                               "Returns a list of all mitochondrial sections")
        .def_property_readonly("root_sections", &morphio::Mitochondria::rootSections,
                               "Returns a list of all root sections (section whose parent ID is -1)");


    py::class_<morphio::Soma>(m, "Soma")
        .def(py::init<const morphio::Soma&>())
        .def_property_readonly("points", [](morphio::Soma* soma){ return span_array_to_ndarray(soma->points()); },
                               "Returns the coordinates (x,y,z) of all soma point")
        .def_property_readonly("diameters", [](morphio::Soma* soma){ return span_to_ndarray(soma->diameters()); },
                               "Returns the diameters of all soma points")

        .def_property_readonly("center", [](morphio::Soma* soma){
                                             return py::array(3, soma->center().data());
                                         },
            "Returns the center of gravity of the soma points")
        .def_property_readonly("type",
                               &morphio::Soma::type,
                               "Returns the soma type")

        .def_property_readonly("surface", &morphio::Soma::surface,
                               "Returns the soma surface\n\n"
                               "Note: the soma surface computation depends on the soma type");

    py::class_<morphio::Section>(m, "Section")
        // Topology-related member functions
        .def_property_readonly("parent", &morphio::Section::parent,
                               "Returns the parent section of this section\n"
                               "throw MissingParentError is the section doesn't have a parent")
        .def_property_readonly("is_root", &morphio::Section::isRoot,
                               "Returns true if this section is a root section (parent ID == -1)")
        .def_property_readonly("children", &morphio::Section::children,
                               "Returns a list of children sections")

        // Property-related accessors
        .def_property_readonly("id", &morphio::Section::id,
                               "Returns the section ID\n"
                               "The section ID can be used to query sections via Morphology::section(uint32_t id)")
        .def_property_readonly("type", &morphio::Section::type,
                               "Returns the morphological type of this section "
                               "(dendrite, axon, ...)")
        .def_property_readonly("points", [](morphio::Section* section){ return span_array_to_ndarray(section->points()); },
                               "Returns list of section's point coordinates")
        .def_property_readonly("diameters", [](morphio::Section* section){ return span_to_ndarray(section->diameters()); },
                               "Returns list of section's point diameters")
        .def_property_readonly("perimeters", [](morphio::Section* section){ return span_to_ndarray(section->perimeters()); },
                               "Returns list of section's point perimeters")

        // Iterators
        .def("iter", [](morphio::Section* section, morphio::IterType type) {
                switch (type) {
                case morphio::IterType::DEPTH_FIRST:
                    return py::make_iterator(section->depth_begin(), section->depth_end());
                case morphio::IterType::BREADTH_FIRST:
                    return py::make_iterator(section->breadth_begin(), section->breadth_end());
                case morphio::IterType::UPSTREAM:
                    return py::make_iterator(section->upstream_begin(), section->upstream_end());
                default:
                    LBTHROW(morphio::MorphioError("Only iteration types depth_first, breadth_first and upstream are supported"));
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator\n"
            "\n"
            "iter_type controls the iteration order. 3 values can be passed:\n"
            "- morphio.IterType.depth_first (default)\n"
            "- morphio.IterType.breadth_first\n"
            "- morphio.IterType.upstream\n",
            "iter_type"_a=morphio::IterType::DEPTH_FIRST);


    py::class_<morphio::MitoSection>(m, "MitoSection")
        // Topology-related member functions
        .def_property_readonly("parent", &morphio::MitoSection::parent,
                               "Returns the parent mitochondrial section of this section\n"
                               "throw MissingParentError is the section doesn't have a parent")
        .def_property_readonly("is_root", &morphio::MitoSection::isRoot,
                               "Returns true if this section is a root section (parent ID == -1)")
        .def_property_readonly("children", &morphio::MitoSection::children,
                               "Returns a list of children mitochondrial sections")

        // Property-related accesors
        .def_property_readonly("id", &morphio::MitoSection::id,
                               "Returns the section ID\n"
                               "The section ID can be used to query sections via Mitochondria::section(uint32_t id)")
        .def_property_readonly("neurite_section_ids", [](morphio::MitoSection* section){ return span_to_ndarray(section->neuriteSectionIds()); },
                               "Returns list of neuronal section IDs associated to each point "
                               "of this mitochondrial section")
        .def_property_readonly("diameters", [](morphio::MitoSection* section){ return span_to_ndarray(section->diameters()); },
                               "Returns list of section's point diameters")
        .def_property_readonly("relative_path_lengths", [](morphio::MitoSection* section){ return span_to_ndarray(section->relativePathLengths()); },
                               "Returns list of relative distances between the start of the "
                               "neuronal section and each point of the mitochondrial section\n\n"
                               "Note: - a relative distance of 0 means the mitochondrial point is at the "
                               "beginning of the neuronal section\n"
                               "      - a relative distance of 1 means the mitochondrial point is at the "
                               "end of the neuronal section\n")

        // Iterators
        .def("iter", [](morphio::MitoSection* section, morphio::IterType type) {
                switch (type) {
                case morphio::IterType::DEPTH_FIRST:
                    return py::make_iterator(section->depth_begin(), section->depth_end());
                case morphio::IterType::BREADTH_FIRST:
                    return py::make_iterator(section->breadth_begin(), section->breadth_end());
                case morphio::IterType::UPSTREAM:
                    return py::make_iterator(section->upstream_begin(), section->upstream_end());
                default:
                    LBTHROW(morphio::MorphioError("Only iteration types depth_first, breadth_first and upstream are supported"));
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "iter_type"_a=morphio::IterType::DEPTH_FIRST);

}
