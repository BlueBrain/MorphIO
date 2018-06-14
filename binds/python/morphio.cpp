#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <morphio/types.h>
#include <morphio/enums.h>

#include <morphio/mito_iterators.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/mito_section.h>
#include <morphio/soma.h>
#include <morphio/errorMessages.h>

#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>

namespace py = pybind11;
using namespace py::literals;


py::array_t<float> span_array_to_ndarray(const morphio::range<const std::array<float, 3> > &span)
{
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        (void*)span.data(),                            /* Pointer to buffer */
        sizeof(float),                          /* Size of one scalar */
        py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
        2,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        { (int) span.size(), 3 }, /* buffer dimentions */
        { sizeof(float) * 3,                  /* Strides (in bytes) for each index */
                sizeof(float) }
        );
    return py::array(buffer_info);
}

template <typename T>
py::array_t<float> span_to_ndarray(const morphio::range<const T>& span)
{
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        (void*)span.data(),                            /* Pointer to buffer */
        sizeof(T),                          /* Size of one scalar */
        py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
        1,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        {(int) span.size()}, /* buffer dimentions */
        {sizeof(T)} );      /* Strides (in bytes) for each index */
    return py::array(buffer_info);
}


morphio::Points array_to_points(py::array_t<float> &buf){
    morphio::Points points;
    py::buffer_info info = buf.request();
    float* ptr = (float*)info.ptr;

    for(int i = 0;i<info.shape[0]; ++i){
        points.push_back(std::array<float, 3>{ptr[3 * i],
                    ptr[3 * i + 1],
                    ptr[3 * i + 2]});
    }
    return points;
}

PYBIND11_MODULE(morphio, m) {

    m.def("set_maximum_warnings", &morphio::set_maximum_warnings,
          "Set the maximum number of warnings to be printed on screen");
    py::enum_<morphio::enums::AnnotationType>(m, "AnnotationType")
        .value("single_child", morphio::enums::AnnotationType::SINGLE_CHILD,
            "Indicates that a section has only one child");

    py::enum_<morphio::enums::IterType>(m, "IterType")
        .value("depth_first", morphio::enums::IterType::DEPTH_FIRST)
        .value("breadth_first", morphio::enums::IterType::BREADTH_FIRST)
        .value("upstream", morphio::enums::IterType::UPSTREAM)
        .export_values();

    py::enum_<morphio::enums::SectionType>(m, "SectionType")
        .value("undefined", morphio::enums::SectionType::SECTION_UNDEFINED)
        .value("soma", morphio::enums::SectionType::SECTION_SOMA)
        .value("axon", morphio::enums::SectionType::SECTION_AXON)
        .value("basal_dendrite", morphio::enums::SectionType::SECTION_DENDRITE)
        .value("apical_dendrite", morphio::enums::SectionType::SECTION_APICAL_DENDRITE)
        // .value("glia_process", morphio::enums::SectionType::SECTION_GLIA_PROCESS)
        // .value("glia_endfoot", morphio::enums::SectionType::SECTION_GLIA_ENDFOOT)
        .export_values();

    py::enum_<morphio::enums::MorphologyVersion>(m, "MorphologyVersion")
        .value("MORPHOLOGY_VERSION_H5_1", morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1)
        .value("MORPHOLOGY_VERSION_H5_2", morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_2)
        .value("MORPHOLOGY_VERSION_H5_1_1", morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1_1)
        .value("MORPHOLOGY_VERSION_SWC_1", morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_SWC_1)
        .value("MORPHOLOGY_VERSION_UNDEFINED", morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_UNDEFINED)
        .export_values();

    py::enum_<morphio::enums::CellFamily>(m, "CellFamily")
        .value("FAMILY_NEURON", morphio::enums::CellFamily::FAMILY_NEURON)
        .value("FAMILY_GLIA", morphio::enums::CellFamily::FAMILY_GLIA)
        .export_values();


    py::enum_<morphio::enums::AccessMode>(m, "AccessMode")
        .value("MODE_READ", morphio::enums::AccessMode::MODE_READ)
        .value("MODE_WRITE", morphio::enums::AccessMode::MODE_WRITE)
        .value("MODE_OVERWRITE", morphio::enums::AccessMode::MODE_OVERWRITE)
        .value("MODE_READWRITE", morphio::enums::AccessMode::MODE_READWRITE)
        .value("MODE_READOVERWRITE", morphio::enums::AccessMode::MODE_READOVERWRITE)
        .export_values();


    py::enum_<morphio::enums::SomaType>(m, "SomaType")
        .value("SOMA_UNDEFINED", morphio::enums::SomaType::SOMA_UNDEFINED)
        .value("SOMA_SINGLE_POINT", morphio::enums::SomaType::SOMA_SINGLE_POINT)
        .value("SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS", morphio::enums::SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
        .value("SOMA_CYLINDERS", morphio::enums::SomaType::SOMA_CYLINDERS)
        .value("SOMA_THREE_POINTS", morphio::enums::SomaType::SOMA_THREE_POINTS)
        .value("SOMA_SIMPLE_CONTOUR", morphio::enums::SomaType::SOMA_SIMPLE_CONTOUR);


    auto base = py::register_exception<morphio::MorphioError&>(m, "MorphioError");
    // base.ptr() signifies "inherits from"
    auto raw = py::register_exception<morphio::RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<morphio::UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<morphio::SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<morphio::IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<morphio::MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<morphio::MissingParentError&>(m, "MissingParentError", raw.ptr());
    py::register_exception<morphio::SectionBuilderError&>(m, "SectionBuilderError", raw.ptr());


    py::class_<morphio::Points>(m, "Points", py::buffer_protocol())
        .def_buffer([](morphio::Points &m) -> py::buffer_info {
                return py::buffer_info(
                    m.data(),                               /* Pointer to buffer */
                    sizeof(float),                          /* Size of one scalar */
                    py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                    2,                                      /* Number of dimensions */
                    { (int)  m.size(), 3 },                 /* Buffer dimensions */
                    { sizeof(float) * 3,             /* Strides (in bytes) for each index */
                            sizeof(float) }
                    );
            });


    m.doc() = "pybind11 example plugin"; // optional module docstring

  // http://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=iostream#capturing-standard-output-from-ostream
    py::add_ostream_redirect(m, "ostream_redirect");

    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const morphio::URI&>())
        .def(py::init<const morphio::mut::Morphology&>())
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
        .def(py::self != py::self)


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
                               "Returns a vector containing all section objects\n\n"
                               "Note: the first section of the vector is always the soma section")

        .def("section", &morphio::Morphology::section,
             "Returns the Section with the given id\n"
             "Reminder: ID = 0 is the soma section\n\n"
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
            "Returns a list with all diameters from all sections")
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
                               "Returns the version");


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
            "Returns the center of gravity of the soma points");


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
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
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
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "iter_type"_a=morphio::IterType::DEPTH_FIRST);



    ////////////////////////////////////////////////////////////////////////////////
    //       Mutable module
    ////////////////////////////////////////////////////////////////////////////////
    py::module mut_module = m.def_submodule("mut");

    auto mutable_morphology = py::class_<morphio::mut::Morphology>(mut_module, "Morphology")
        .def(py::init<>())
        .def(py::init<const morphio::URI&>())
        .def(py::init<const morphio::Morphology&>())

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
        .def("is_root", &morphio::mut::Morphology::isRoot,
             "Return True if section is a root section", "section_id"_a)
        .def("parent", &morphio::mut::Morphology::parent,
             "Get the parent ID\n\n"
             "Note: Root sections return -1",
             "section_id"_a)
        .def("children", &morphio::mut::Morphology::children,
             "Returns a list of children IDs",
             "section_id"_a)
        .def("section", &morphio::mut::Morphology::section,
             "Returns the section with the given id\n\n"
             "Note: multiple morphologies can share the same Section instances",
             "section_id"_a)
        .def("build_read_only", (const morphio::Property::Properties (morphio::mut::Morphology::*)() const) &morphio::mut::Morphology::buildReadOnly,
             "Returns the data structure used to create read-only morphologies")
        .def("append_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*) (std::shared_ptr<morphio::mut::Section>, const morphio::Property::PointLevel&, morphio::SectionType)) &morphio::mut::Morphology::appendSection,
             "Append a new Section the given parentId (None appends to soma)\n"
             " If section_type is omitted or set to 'undefined'"
             " the type of the parent section will be used"
             " (Root sections can't have sectionType ommited)",
             "parent_id"_a, "point_level_properties"_a, "section_type"_a=morphio::SectionType::SECTION_UNDEFINED)
        .def("append_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*) (std::shared_ptr<morphio::mut::Section>, const morphio::Section&, bool)) &morphio::mut::Morphology::appendSection,
             "Append the existing immutable Section to the given parentId (None appends to soma) "
             "If recursive == true, all descendent will be appended as well",
             "parent_id"_a, "immutable_section"_a, "recursive"_a=false)

        .def("delete_section", &morphio::mut::Morphology::deleteSection,
             "Delete the given section\n"
             "\n"
             "Will silently fail if the section id is not part of the tree\n"
             "\n"
             "If recursive == true, all descendent sections will be deleted as well\n"
             "Else, children will be re-attached to their grand-parent",
             "section_id"_a, "recursive"_a=true)

        .def_property_readonly("cell_family", &morphio::mut::Morphology::cellFamily,
                               "Returns the cell family (neuron or glia)")

        .def_property_readonly("soma_type", &morphio::mut::Morphology::somaType,
                               "Returns the soma type")

        .def_property_readonly("version", &morphio::mut::Morphology::version,
                               "Returns the version")

        .def("write", &morphio::mut::Morphology::write,
             "Write file to H5, SWC, ASC format depending on filename extension", "filename"_a)

        // Iterators
        .def("iter", [](morphio::mut::Morphology* morph, std::shared_ptr<morphio::mut::Section> section, morphio::IterType type) {
                switch (type) {
                case morphio::IterType::DEPTH_FIRST:
                    return py::make_iterator(morph->depth_begin(section), morph->depth_end());
                case morphio::IterType::BREADTH_FIRST:
                    return py::make_iterator(morph->breadth_begin(section), morph->breadth_end());
                case morphio::IterType::UPSTREAM:
                    return py::make_iterator(morph->upstream_begin(section), morph->upstream_end());
                }
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "id"_a=-1, "iter_type"_a=morphio::IterType::DEPTH_FIRST);


    mutable_morphology.def("append_section", (std::shared_ptr<morphio::mut::Section> (morphio::mut::Morphology::*) (std::shared_ptr<morphio::mut::Section>, std::shared_ptr<morphio::mut::Section>, const morphio::mut::Morphology&)) &morphio::mut::Morphology::appendSection,
             "Append the existing mutable Section to the given parentId (None appends to soma) "
             "If a mut::morphio::Morphology is passed, all descendent of section in this "
             "morphology will be appended as well"
             "parent_id"_a, "mutable_section"_a, "morphology"_a=morphio::mut::Morphology());

    py::class_<morphio::mut::Mitochondria>(mut_module, "Mitochondria")
        .def(py::init<>())
        .def_property_readonly("root_sections", &morphio::mut::Mitochondria::rootSections,
                               "Returns a list of all root sections IDs "
                               "(sections whose parent ID are -1)")
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
        .def("append_section", (uint32_t (morphio::mut::Mitochondria::*) (int32_t, const morphio::Property::MitochondriaPointLevel&)) &morphio::mut::Mitochondria::appendSection,
             "Append a new MitoSection the given parentId (-1 create a new mitochondrion)",
             "parent_id"_a, "point_level_properties"_a)
        .def("depth_begin", [](morphio::mut::Mitochondria* morph, int32_t id) {
                return py::make_iterator(morph->depth_begin(id), morph->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Depth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a=-1)
        .def("breadth_begin", [](morphio::mut::Mitochondria* morph, int32_t id) {
                return py::make_iterator(morph->breadth_begin(id), morph->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Breadth first iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a=-1)
        .def("upstream_begin", [](morphio::mut::Mitochondria* morph, int32_t id) {
                return py::make_iterator(morph->upstream_begin(id), morph->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Upstream iterator starting at a given section id\n"
            "\n"
            "If id == -1, the iteration will be successively performed starting\n"
            "at each root section",
            "section_id"_a=-1);



    // py::nodelete needed because morphio::mut::MitoSection has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::MitoSection, std::unique_ptr<morphio::mut::MitoSection, py::nodelete>>(mut_module, "MitoSection")
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
                      "Returns the neurite section Ids of all points of this section");

    // py::nodelete needed because morphio::mut::Section has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::Section, std::shared_ptr<morphio::mut::Section>>(mut_module, "Section")
        .def_property_readonly("id", &morphio::mut::Section::id,
                               "Return the section ID")
        .def_property("type",
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
                      "Returns the perimeters of all points of this section");

    // py::nodelete needed because morphio::mut::Soma has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::Soma, std::unique_ptr<morphio::mut::Soma, py::nodelete>>(mut_module, "Soma")
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




    py::class_<morphio::Property::Properties>(m, "Properties",
                                              "The higher level container structure is Property::Properties"
        )
        .def_readwrite("point_level", &morphio::Property::Properties::_pointLevel,
                       "Returns the structure that stores information at the point level")
        .def_readwrite("section_level", &morphio::Property::Properties::_sectionLevel,
                       "Returns the structure that stores information at the section level")
        .def_readwrite("cell_level", &morphio::Property::Properties::_cellLevel,
                       "Returns the structure that stores information at the cell level");


    py::class_<morphio::Property::PointLevel>(m, "PointLevel",
                                              "Container class for information available at the point level (point coordinate, diameter, perimeter)")
        .def(py::init<>())
        .def(py::init<std::vector<morphio::Property::Point::Type>,
             std::vector<morphio::Property::Diameter::Type>>(),
             "points"_a, "diameters"_a)
        .def(py::init<std::vector<morphio::Property::Point::Type>,
             std::vector<morphio::Property::Diameter::Type>,
             std::vector<morphio::Property::Perimeter::Type>>(),
             "points"_a, "diameters"_a, "perimeters"_a)
        .def_readwrite("points",
                       &morphio::Property::PointLevel::_points,
                       "Returns the list of point coordinates")
        .def_readwrite("perimeters", &morphio::Property::PointLevel::_perimeters,
                       "Returns the list of perimeters")
        .def_readwrite("diameters", &morphio::Property::PointLevel::_diameters,
                       "Returns the list of diameters");

    py::class_<morphio::Property::SectionLevel>(m, "SectionLevel",
                                                "Container class for information available at the section level (section type, parent section)")
        .def_readwrite("sections", &morphio::Property::SectionLevel::_sections,
                       "Returns a list of [offset, parent section ID]")
        .def_readwrite("section_types", &morphio::Property::SectionLevel::_sectionTypes,
                       "Returns the list of section types")
        .def_readwrite("children", &morphio::Property::SectionLevel::_children,
                       "Returns a dictionary where key is a section ID "
                       "and value is the list of children section IDs");

    py::class_<morphio::Property::CellLevel>(m, "CellLevel",
                                             "Container class for information available at the cell level (cell type, file version, soma type)")
        .def_readwrite("cell_family", &morphio::Property::CellLevel::_cellFamily,
                       "Returns the cell family (neuron or glia)")
        .def_readwrite("soma_type", &morphio::Property::CellLevel::_somaType,
                       "Returns the soma type")
        .def_readwrite("version", &morphio::Property::CellLevel::_version,
                       "Returns the version");

    py::class_<morphio::Property::Annotation>(m, "Annotation",
                                             "Container class for information about anomalies detected while parsing the file (no soma, section with a single child...)")
        .def_readwrite("type", &morphio::Property::Annotation::_type,
                               "Returns the type")
        .def_readwrite("section_id", &morphio::Property::Annotation::_sectionId,
                       "Returns the sectionId")
        .def_readwrite("line_number", &morphio::Property::Annotation::_lineNumber,
                       "Returns the lineNumber")
        .def_readwrite("details", &morphio::Property::Annotation::_details,
                               "Returns the details")
        .def_property_readonly("points",
                       [](morphio::Property::Annotation* a) {
                           return a->_points._points;
                       },
                       "Returns the list of coordinates of annotated points")
        .def_property_readonly("diameters", [](morphio::Property::Annotation* a) {
                return a->_points._diameters;
            },
            "Returns the list of diameters of annotated points")
        .def_property_readonly("perimeters",
                               [](morphio::Property::Annotation* a) {
                                   return a->_points._perimeters;
                               },
                               "Returns the list of perimeters of annotated points");


    py::class_<morphio::Property::MitochondriaPointLevel>(m, "MitochondriaPointLevel",
                                                          "Container class for the information available at the mitochondrial point level (enclosing neuronal section, relative distance to start of neuronal section, diameter)")
        .def(py::init<>())
        .def(py::init<std::vector<uint32_t>, std::vector<float>,
             std::vector<morphio::Property::Diameter::Type>>(),
             "neuronal_section_ids"_a, "distances_to_section_start"_a, "diameters"_a);
}
