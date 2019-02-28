#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>

namespace py = pybind11;
using namespace py::literals;

void bind_mutable_module(py::module &m) {

    auto mutable_morphology = py::class_<morphio::mut::Morphology>(m, "Morphology")
        .def(py::init<>())
        .def(py::init<const morphio::URI&, unsigned int>(),
             "filename"_a, "options"_a=morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const morphio::Morphology&, unsigned int>(),
             "morphology"_a, "options"_a=morphio::enums::Option::NO_MODIFIER)
        .def(py::init<const morphio::mut::Morphology&, unsigned int>(),
             "morphology"_a, "options"_a=morphio::enums::Option::NO_MODIFIER)

        .def("__eq__", [](const morphio::mut::Morphology& a, const morphio::mut::Morphology& b) {
                return a.operator==(b);
            }, py::is_operator(),
            "Are considered equal, 2 morphologies with the same:\n"
            "- root sections\n"
            "- section topology\n"
            "- cell family\n"
            "For each section:"
            "- same points\n"
            "- same diameters\n"
            "- same perimeters\n"
            "- same type\n"
            "Note: the soma types are NOT required to be equal")
        .def("__ne__", [](const morphio::mut::Morphology& a, const morphio::mut::Morphology& b) {
                return a.operator!=(b);
            }, py::is_operator())

        // Cell sub-part accessors
        .def_property_readonly("sections", &morphio::mut::Morphology::sections,
                               "Returns a list containing IDs of all sections. "
                               "The first section of the vector is the soma section")
        .def_property_readonly("root_sections", &morphio::mut::Morphology::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)", py::return_value_policy::reference)
        .def_property_readonly("soma",
                               (std::shared_ptr<morphio::mut::Soma>(morphio::mut::Morphology::*)())&morphio::mut::Morphology::soma,
                               "Returns a reference to the soma object\n\n"
                               "Note: multiple morphologies can share the same Soma instance")
        .def_property_readonly("mitochondria", (morphio::mut::Mitochondria& (morphio::mut::Morphology::*) ())
                               &morphio::mut::Morphology::mitochondria,
                               "Returns a reference to the mitochondria container class")
        .def_property_readonly("annotations", &morphio::mut::Morphology::annotations,
                               "Returns a list of annotations")
        .def("section", &morphio::mut::Morphology::section,
             "Returns the section with the given id\n\n"
             "Note: multiple morphologies can share the same Section instances",
             "section_id"_a)
        .def("build_read_only", (const morphio::Property::Properties (morphio::mut::Morphology::*)() const) &morphio::mut::Morphology::buildReadOnly,
             "Returns the data structure used to create read-only morphologies")
        .def("append_root_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*) (const morphio::Property::PointLevel&, morphio::SectionType)) &morphio::mut::Morphology::appendRootSection,
             "Append a root Section\n",
             "point_level_properties"_a, "section_type"_a)
        .def("append_root_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*) (const morphio::Section&, bool)) &morphio::mut::Morphology::appendRootSection,
             "Append the existing immutable Section as a root section\n"
             "If recursive == true, all descendent will be appended as well",
             "immutable_section"_a, "recursive"_a=false)

        .def("delete_section", &morphio::mut::Morphology::deleteSection,
             "Delete the given section\n"
             "\n"
             "Will silently fail if the section id is not part of the tree\n"
             "\n"
             "If recursive == true, all descendent sections will be deleted as well\n"
             "Else, children will be re-attached to their grand-parent",
             "section_id"_a, "recursive"_a=true)

        .def("as_immutable", [](const morphio::mut::Morphology* morph) { return morphio::Morphology(*morph); })

        .def_property_readonly("cell_family", &morphio::mut::Morphology::cellFamily,
                               "Returns the cell family (neuron or glia)")

        .def_property_readonly("soma_type", &morphio::mut::Morphology::somaType,
                               "Returns the soma type")

        .def_property_readonly("version", &morphio::mut::Morphology::version,
                               "Returns the version")

        .def("write", &morphio::mut::Morphology::write,
             "Write file to H5, SWC, ASC format depending on filename extension", "filename"_a)

        // Iterators
        .def("iter", [](morphio::mut::Morphology* morph, morphio::IterType type) {
                switch (type) {
                case morphio::IterType::DEPTH_FIRST:
                    return py::make_iterator(morph->depth_begin(), morph->depth_end());
                case morphio::IterType::BREADTH_FIRST:
                    return py::make_iterator(morph->breadth_begin(), morph->breadth_end());
                default:
                LBTHROW(morphio::MorphioError("Only iteration types depth_first and breadth_first are supported"));
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator that runs successively on every neurite\n"
            "iter_type controls the order of iteration on sections of a given neurite. 2 values can be passed:\n"
            "- morphio.IterType.depth_first (default)\n"
            "- morphio.IterType.breadth_first",
            "iter_type"_a=morphio::IterType::DEPTH_FIRST);


    mutable_morphology.def("append_root_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*)
                                              (std::shared_ptr<morphio::mut::Section>, bool))
                           &morphio::mut::Morphology::appendRootSection,
                           "Append the existing mutable Section as a root section\n"
                           "If recursive == true, all descendent will be appended as well",
                           "mutable_section"_a, "recursive"_a=false);

    py::class_<morphio::mut::Mitochondria>(m, "Mitochondria")
        .def(py::init<>())
        .def_property_readonly("root_sections", &morphio::mut::Mitochondria::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)", py::return_value_policy::reference)
        .def_property_readonly("sections", &morphio::mut::Mitochondria::sections,
                               "Return a dict where key is the mitochondrial section ID"
                               " and value is the mithochondrial section")
        .def("is_root", &morphio::mut::Mitochondria::isRoot,
             "Return True if section is a root section", "section_id"_a)
        .def("parent", &morphio::mut::Mitochondria::parent,
             "Returns the parent mithochondrial section ID",
             "section_id"_a)
        .def("children", &morphio::mut::Mitochondria::children,
             "section_id"_a)
        .def("section", &morphio::mut::Mitochondria::section,
             "Get a reference to the given mithochondrial section\n\n"
             "Note: multiple mitochondria can shared the same references",
             "section_id"_a)
        .def("append_root_section", (std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::Mitochondria::*)
                                     (const morphio::Property::MitochondriaPointLevel&))
             &morphio::mut::Mitochondria::appendRootSection,
             "Append a new root MitoSection",
             "point_level_properties"_a)
        .def("append_root_section", (std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::Mitochondria::*)
                                     (const morphio::MitoSection&, bool recursive))
             &morphio::mut::Mitochondria::appendRootSection,
             "Append a new root MitoSection (if recursive == true, all descendent will be appended as well)",
             "immutable_section"_a, "recursive"_a = true)
        .def("append_root_section", (std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::Mitochondria::*)
                                     (const std::shared_ptr<morphio::mut::MitoSection>, bool recursive))
             &morphio::mut::Mitochondria::appendRootSection,
             "Append a new root MitoSection (if recursive == true, all descendent will be appended as well)",
             "section"_a, "recursive"_a = true)


        .def("depth_begin", [](morphio::mut::Mitochondria* morph, std::shared_ptr<morphio::mut::MitoSection> section) {
                return py::make_iterator(morph->depth_begin(section), morph->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a=-1)
        .def("breadth_begin", [](morphio::mut::Mitochondria* morph, std::shared_ptr<morphio::mut::MitoSection> section) {
                return py::make_iterator(morph->breadth_begin(section), morph->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Breadth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a=-1)
        .def("upstream_begin", [](morphio::mut::Mitochondria* morph, std::shared_ptr<morphio::mut::MitoSection> section) {
                return py::make_iterator(morph->upstream_begin(section), morph->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Upstream iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a=-1);



    // py::nodelete needed because morphio::mut::MitoSection has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::MitoSection, std::shared_ptr<morphio::mut::MitoSection>>(m, "MitoSection")
        .def_property_readonly("id", &morphio::mut::MitoSection::id,
                               "Return the section ID")
        .def_property("diameters",
                      &morphio::mut::MitoSection::diameters,
                      [](morphio::mut::MitoSection* section,
                         const std::vector<float>& _diameters) {
                          section -> diameters() = _diameters;
                      },
                      "Returns the diameters of all points of this section")
        .def_property("relative_path_lengths",
                      &morphio::mut::MitoSection::pathLengths,
                      [](morphio::mut::MitoSection* section,
                         const std::vector<float>& _pathLengths) {
                          section -> pathLengths() = _pathLengths;
                      },
                      "Returns the relative distance (between 0 and 1)\n"
                      "between the start of the neuronal section and each point\n"
                      "of this mitochondrial section")
        .def_property("neurite_section_ids",
                      &morphio::mut::MitoSection::neuriteSectionIds,
                      [](morphio::mut::MitoSection* section,
                         const std::vector<uint32_t>& _neuriteSectionIds) {
                          section -> neuriteSectionIds() = _neuriteSectionIds;
                      },
                      "Returns the neurite section Ids of all points of this section")

        .def("append_section", (std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::MitoSection::*) (const morphio::Property::MitochondriaPointLevel&)) &morphio::mut::MitoSection::appendSection,
             "Append a new MitoSection to this mito section",
             "point_level_properties"_a)

        .def("append_section", (std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::MitoSection::*) (std::shared_ptr<morphio::mut::MitoSection>, bool)) &morphio::mut::MitoSection::appendSection,
             "Append a copy of the section to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "section"_a, "recursive"_a=false)

        .def("append_section", (std::shared_ptr<morphio::mut::MitoSection> (morphio::mut::MitoSection::*) (const morphio::MitoSection&, bool)) &morphio::mut::MitoSection::appendSection,
             "Append the existing immutable MitoSection to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "immutable_section"_a, "recursive"_a=false);

    py::class_<morphio::mut::Section, std::shared_ptr<morphio::mut::Section>>(m, "Section")
        .def_property_readonly("id", &morphio::mut::Section::id,
                               "Return the section ID")
        .def_property("type",
                      (const morphio::SectionType& (morphio::mut::Section::*)() const)
                      &morphio::mut::Section::type,
                      [](morphio::mut::Section* section,
                         morphio::SectionType _type) {
                          section -> type() = _type;
                      },
                      "Returns the morphological type of this section "
                      "(dendrite, axon, ...)")
        .def_property("points",
                      [](morphio::mut::Section* section){
                          return py::array(section->points().size(),
                                           section->points().data());
                      },
                      [](morphio::mut::Section* section,
                         py::array_t<float> _points) {
                          section -> points() = array_to_points(_points);
                      },
                      "Returns the coordinates (x,y,z) of all points of this section")
        .def_property("diameters",
                      [](morphio::mut::Section* section){
                          return py::array(section->diameters().size(),
                                           section->diameters().data());
                      },
                      [](morphio::mut::Section* section,
                         py::array_t<float> _diameters) {
                          section -> diameters() = _diameters.cast<std::vector<float>>();
                      },
                      "Returns the diameters of all points of this section")
        .def_property("perimeters",
                      [](morphio::mut::Section* section){
                          return py::array(section->perimeters().size(),
                                           section->perimeters().data());
                      },
                      [](morphio::mut::Section* section,
                         py::array_t<float> _perimeters) {
                          section -> perimeters() = _perimeters.cast<std::vector<float>>();
                      },
                      "Returns the perimeters of all points of this section")
        .def_property_readonly("is_root", &morphio::mut::Section::isRoot,
             "Return True if section is a root section")
        .def_property_readonly("parent", &morphio::mut::Section::parent,
             "Get the parent ID\n\n"
             "Note: Root sections return -1")
        .def_property_readonly("children", &morphio::mut::Section::children,
             "Returns a list of children IDs")
        // Iterators
        .def("iter", [](morphio::mut::Section* section, morphio::IterType type) {
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
            "iter_type"_a=morphio::IterType::DEPTH_FIRST)

        // Editing
        .def("append_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Section::*) (const morphio::Section&, bool)) &morphio::mut::Section::appendSection,
             "Append the existing immutable Section to this section"
             "If recursive == true, all descendent will be appended as well",
             "immutable_section"_a, "recursive"_a=false)

        .def("append_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Section::*) (std::shared_ptr<morphio::mut::Section>, bool)) &morphio::mut::Section::appendSection,
             "Append the existing mutable Section to this section\n"
             "If recursive == true, all descendent will be appended as well",
             "mutable_section"_a, "recursive"_a=false)

        .def("append_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Section::*) (const morphio::Property::PointLevel&, morphio::SectionType)) &morphio::mut::Section::appendSection,
             "Append a new Section to this section\n"
             " If section_type is omitted or set to 'undefined'"
             " the type of the parent section will be used",
             "point_level_properties"_a, "section_type"_a=morphio::SectionType::SECTION_UNDEFINED);



    py::class_<morphio::mut::Soma, std::shared_ptr<morphio::mut::Soma>>(m, "Soma")
        .def(py::init<const morphio::Property::PointLevel&>())
        .def_property("points",
                      [](morphio::mut::Soma* soma){
                          return py::array(soma->points().size(),
                                           soma->points().data());
                      },
                      [](morphio::mut::Soma* soma,
                         py::array_t<float> _points) {
                          soma -> points() = array_to_points(_points);
                      },
                      "Returns the coordinates (x,y,z) of all soma point")
        .def_property("diameters",
                      [](morphio::mut::Soma* soma){
                          return py::array(soma->diameters().size(),
                                           soma->diameters().data());
                      },
                      [](morphio::mut::Soma* soma,
                         py::array_t<float> _diameters) {
                          soma -> diameters() = _diameters.cast<std::vector<float>>();
                      },
                      "Returns the diameters of all soma points")
        .def_property_readonly("type",
                               &morphio::mut::Soma::type,
                               "Returns the soma type")
        .def_property_readonly("surface",
                               &morphio::mut::Soma::surface,
                               "Returns the soma surface\n\n"
                               "Note: the soma surface computation depends on the soma type")
        .def_property_readonly("center", [](morphio::mut::Soma* soma){
                return py::array(3, soma->center().data());
            },
            "Returns the center of gravity of the soma points");
}
