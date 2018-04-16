#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
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


PYBIND11_MODULE(morphio, m) {

    m.doc() = "pybind11 example plugin"; // optional module docstring

    http://pybind11.readthedocs.io/en/stable/advanced/pycpp/utilities.html?highlight=iostream#capturing-standard-output-from-ostream
    py::add_ostream_redirect(m, "ostream_redirect");

    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const morphio::URI&>())
        .def(py::init<const morphio::mut::Morphology&>())
        .def("__eq__", [](const morphio::Morphology& a, const morphio::Morphology& b) {
                return a.operator==(b);
            }, py::is_operator())

        // Cell sub-parts
        .def_property_readonly("soma", &morphio::Morphology::soma,
                               "Return the soma object")
        .def_property_readonly("mitochondria", &morphio::Morphology::mitochondria,
                               "Return the soma object")
        .def_property_readonly("root_sections", &morphio::Morphology::rootSections,
                               "Return a list of all root sections "
                               "(sections whose parent ID are -1)")
        .def_property_readonly("sections", &morphio::Morphology::sections,
                               "Return a vector containing all section objects. "
                               "The first section of the vector is the soma section")
        .def("section", &morphio::Morphology::section,
             "Return the Section with the given id\n"
             "Reminder: ID = 0 is the soma section\n\n"
             "throw RawDataError if the id is out of range",
            "section_id"_a)

        // Property accessors
        .def_property_readonly("points", &morphio::Morphology::points,
                               "Return a list with all points from all sections")
        .def_property_readonly("diameters", &morphio::Morphology::diameters,
                               "Return a list with all diameters from all sections")
        .def_property_readonly("perimeters", &morphio::Morphology::perimeters,
                               "Return a list with all perimeters from all sections")
        .def_property_readonly("section_types", &morphio::Morphology::sectionTypes,
                               "Return a vector with the section type of every section")
        .def_property_readonly("soma_type", &morphio::Morphology::somaType,
                               "Return the soma type")
        .def_property_readonly("cell_family", &morphio::Morphology::cellFamily,
                               "Return the cell family (neuron or glia)")
        .def_property_readonly("version", &morphio::Morphology::version,
                               "Return the version");

    py::class_<morphio::Mitochondria>(m, "Mitochondria")
        .def("section", &morphio::Mitochondria::section)
        .def_property_readonly("sections", &morphio::Mitochondria::sections)
        .def_property_readonly("root_sections", &morphio::Mitochondria::rootSections);


    py::class_<morphio::Soma>(m, "Soma")
        .def(py::init<const morphio::Soma&>())
        .def_property_readonly("soma_center", &morphio::Soma::somaCenter)
        .def_property_readonly("points", [](morphio::Soma* soma){ return span_array_to_ndarray(soma->points()); })
        .def_property_readonly("diameters", [](morphio::Soma* soma){ return span_to_ndarray(soma->diameters()); });

    py::class_<morphio::Section>(m, "Section")
        .def_property_readonly("parent", &morphio::Section::parent)
        .def_property_readonly("type", &morphio::Section::type)
        .def_property_readonly("is_root", &morphio::Section::isRoot)
        .def_property_readonly("parent", &morphio::Section::parent)
        .def_property_readonly("children", &morphio::Section::children)
        .def_property_readonly("points", [](morphio::Section* section){ return span_array_to_ndarray(section->points()); })
        .def_property_readonly("diameters", [](morphio::Section* section){ return span_to_ndarray(section->diameters()); })
        .def_property_readonly("perimeters", [](morphio::Section* section){ return span_to_ndarray(section->perimeters()); })
        .def_property_readonly("id", &morphio::Section::id)
        .def_property_readonly("depth_begin", [](morphio::Section* section) {
                return py::make_iterator(section->depth_begin(), section->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def_property_readonly("breadth_begin", [](morphio::Section* section) {
                return py::make_iterator(section->breadth_begin(), section->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def_property_readonly("upstream_begin", [](morphio::Section* section) {
                return py::make_iterator(section->upstream_begin(), section->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */);

    py::class_<morphio::MitoSection>(m, "MitoSection")
        .def_property_readonly("parent", &morphio::MitoSection::parent)
        .def_property_readonly("is_root", &morphio::MitoSection::isRoot)
        .def_property_readonly("parent", &morphio::MitoSection::parent)
        .def_property_readonly("children", &morphio::MitoSection::children)
        .def_property_readonly("neurite_section_id", [](morphio::MitoSection* section){ return span_to_ndarray(section->neuriteSectionId()); })
        .def_property_readonly("diameters", [](morphio::MitoSection* section){ return span_to_ndarray(section->diameters()); })
        .def_property_readonly("relative_path_lengths", [](morphio::MitoSection* section){ return span_to_ndarray(section->relativePathLengths()); })
        .def_property_readonly("id", &morphio::MitoSection::id)
        .def_property_readonly("depth_begin", [](morphio::MitoSection* section) {
                return py::make_iterator(section->depth_begin(), section->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def_property_readonly("breadth_begin", [](morphio::MitoSection* section) {
                return py::make_iterator(section->breadth_begin(), section->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def_property_readonly("upstream_begin", [](morphio::MitoSection* section) {
                return py::make_iterator(section->upstream_begin(), section->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */);




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


    ////////////////////////////////////////////////////////////////////////////////
    //       Mutable module
    ////////////////////////////////////////////////////////////////////////////////
    py::module mut_module = m.def_submodule("mut");

    py::class_<morphio::mut::Mitochondria>(mut_module, "Mitochondria")
        .def(py::init<>())
        .def_property_readonly("root_sections", &morphio::mut::Mitochondria::rootSections)
        .def("parent", &morphio::mut::Mitochondria::parent)
        .def("children", &morphio::mut::Mitochondria::children)
        .def("section", &morphio::mut::Mitochondria::section,
             "Get a reference to the given mithochondrial section\n"
             "Note: multiple mitochondria can shared the same references")
        .def("append_section", &morphio::mut::Mitochondria::appendSection);


    py::class_<morphio::mut::Morphology>(mut_module, "Morphology")
        .def(py::init<>())
        .def(py::init<const morphio::URI&>())
        .def(py::init<const morphio::Morphology&>())
        .def_property_readonly("sections", &morphio::mut::Morphology::sections,
                               "Return a list containing IDs of all sections. "
                               "The first section of the vector is the soma section")
        .def_property_readonly("root_sections", &morphio::mut::Morphology::rootSections,
                               "Return a list of all root sections IDs "
                               "(sections whose parent ID are -1)")
        .def_property_readonly("soma",
                               (std::shared_ptr<morphio::mut::Soma>(morphio::mut::Morphology::*)())&morphio::mut::Morphology::soma,
                               "Return a reference to the soma object\n"
                               "Note: multiple morphologies can share the same Soma instance")
        .def("parent", &morphio::mut::Morphology::parent,
             "Get the parent ID\n"
             "Note: Root sections return -1",
             "section_id"_a)
        .def("children", &morphio::mut::Morphology::children,
             "Return a list of children IDs",
             "section_id"_a)
        .def("section", &morphio::mut::Morphology::section,
             "Return the section with the given id
             Note: multiple morphologies can share the same Section instances",
             "section_id"_a)
        .def("mitochondria", (morphio::mut::Mitochondria& (morphio::mut::Morphology::*) ())
             &morphio::mut::Morphology::mitochondria, py::return_value_policy::reference)
        .def("build_read_only", (const morphio::Property::Properties (morphio::mut::Morphology::*)() const) &morphio::mut::Morphology::buildReadOnly)
        .def("delete_section", &morphio::mut::Morphology::deleteSection)
        .def("append_section", (uint32_t (morphio::mut::Morphology::*) (int32_t, morphio::SectionType, const morphio::Property::PointLevel&)) &morphio::mut::Morphology::appendSection)
        .def("delete_section", &morphio::mut::Morphology::deleteSection)

        .def_property_readonly("cell_family", &morphio::mut::Morphology::cellFamily)
        .def_property_readonly("soma_type", &morphio::mut::Morphology::somaType)
        .def_property_readonly("version", &morphio::mut::Morphology::version)

        .def("write_h5", &morphio::mut::Morphology::write_h5)
        .def("write_swc", &morphio::mut::Morphology::write_swc)
        .def("write_asc", &morphio::mut::Morphology::write_asc)

        .def("depth_begin", [](morphio::mut::Morphology* morph, uint32_t id) {
                return py::make_iterator(morph->depth_begin(id), morph->depth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def("breadth_begin", [](morphio::mut::Morphology* morph, uint32_t id) {
                return py::make_iterator(morph->breadth_begin(id), morph->breadth_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
        .def("upstream_begin", [](morphio::mut::Morphology* morph, uint32_t id) {
                return py::make_iterator(morph->upstream_begin(id), morph->upstream_end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */);




        // .def("traverse", &morphio::mut::Morphology::traverse);


    // py::nodelete needed because morphio::mut::MitoSection has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::MitoSection, std::unique_ptr<morphio::mut::MitoSection, py::nodelete>>(mut_module, "MitoSection")
        .def_property("diameters",
                      &morphio::mut::MitoSection::diameters,
                      [](morphio::mut::MitoSection* section,
                         const std::vector<float>& _diameters) {
                          section -> diameters() = _diameters;
                      })
        .def_property("path_lengths",
                      &morphio::mut::MitoSection::pathLengths,
                      [](morphio::mut::MitoSection* section,
                         const std::vector<float>& _pathLengths) {
                          section -> pathLengths() = _pathLengths;
                      })
        .def_property("neurite_section_ids",
                      &morphio::mut::MitoSection::neuriteSectionIds,
                      [](morphio::mut::MitoSection* section,
                         const std::vector<uint32_t>& _neuriteSectionIds) {
                          section -> neuriteSectionIds() = _neuriteSectionIds;
                      });

    // py::nodelete needed because morphio::mut::Section has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::Section, std::unique_ptr<morphio::mut::Section, py::nodelete>>(mut_module, "Section")
        .def_property_readonly("id", &morphio::mut::Section::id)
        .def_property("type",
                      // getter
                      &morphio::mut::Section::type,
                      // setter
                      [](morphio::mut::Section* section,
                         morphio::SectionType _type) {
                          section -> type() = _type;
                      })
        .def_property("points",
                      &morphio::mut::Section::points,
                      [](morphio::mut::Section* section,
                         const std::vector<morphio::Point>& _points) {
                          section -> points() = _points;
                      })
        .def_property("diameters",
                      &morphio::mut::Section::diameters,
                      [](morphio::mut::Section* section,
                         const std::vector<float>& _diameters) {
                          section -> diameters() = _diameters;
                      })
        .def_property("perimeters",
                      &morphio::mut::Section::perimeters,
                      [](morphio::mut::Section* section,
                         const std::vector<float>& _perimeters) {
                          section -> perimeters() = _perimeters;
                      });

    // py::nodelete needed because morphio::mut::Soma has a private destructor
    // http://pybind11.readthedocs.io/en/stable/advanced/classes.html?highlight=private%20destructor#non-public-destructors
    py::class_<morphio::mut::Soma, std::unique_ptr<morphio::mut::Soma, py::nodelete>>(mut_module, "Soma")
        .def(py::init<const morphio::Property::PointLevel&>())
        .def_property("points",
                      (std::vector<morphio::Point>& (morphio::mut::Soma::*) ())
                      &morphio::mut::Soma::points,
                      [](morphio::mut::Soma* soma,
                         const std::vector<morphio::Point>& _points) {
                          soma -> points() = _points;
                      })
        .def_property("diameters",
                      (std::vector<float>& (morphio::mut::Soma::*) ())
                      &morphio::mut::Soma::diameters,
                      [](morphio::mut::Soma* soma,
                         const std::vector<float>& _diameters) {
                          soma -> diameters() = _diameters;
                      });



    py::class_<morphio::Property::MitochondriaPointLevel>(m, "MitochondriaPointLevel")
        .def(py::init<>())
        .def(py::init<std::vector<uint32_t>, std::vector<float>,
             std::vector<morphio::Property::Diameter::Type>>());

    py::class_<morphio::Property::PointLevel>(m, "PointLevel")
        .def(py::init<>())
        .def(py::init<std::vector<morphio::Property::Point::Type>,
             std::vector<morphio::Property::Diameter::Type>>())
        .def(py::init<std::vector<morphio::Property::Point::Type>,
             std::vector<morphio::Property::Diameter::Type>,
             std::vector<morphio::Property::Perimeter::Type>>())
        .def_readwrite("points", &morphio::Property::PointLevel::_points)
        .def_readwrite("perimeters", &morphio::Property::PointLevel::_perimeters)
        .def_readwrite("diameters", &morphio::Property::PointLevel::_diameters);

    py::class_<morphio::Property::SectionLevel>(m, "SectionLevel")
        .def_readwrite("sections", &morphio::Property::SectionLevel::_sections)
        .def_readwrite("section_types", &morphio::Property::SectionLevel::_sectionTypes)
        .def_readwrite("children", &morphio::Property::SectionLevel::_children);

    py::class_<morphio::Property::CellLevel>(m, "CellLevel")
        .def_readwrite("cell_family", &morphio::Property::CellLevel::_cellFamily)
        .def_readwrite("soma_type", &morphio::Property::CellLevel::_somaType)
        .def_readwrite("version", &morphio::Property::CellLevel::_version);

    py::class_<morphio::Property::Properties>(m, "Properties")
        .def_readwrite("point_level", &morphio::Property::Properties::_pointLevel)
        .def_readwrite("section_level", &morphio::Property::Properties::_sectionLevel)
        .def_readwrite("cell_level", &morphio::Property::Properties::_cellLevel);




}
