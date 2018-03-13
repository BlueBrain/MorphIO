#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <morphio/types.h>
#include <morphio/enums.h>

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>

namespace py = pybind11;


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


py::array_t<float> span_to_ndarray(const morphio::range<const float>& span)
{
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        (void*)span.data(),                            /* Pointer to buffer */
        sizeof(float),                          /* Size of one scalar */
        py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
        1,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        {(int) span.size()}, /* buffer dimentions */
        {sizeof(float)} );      /* Strides (in bytes) for each index */
    return py::array(buffer_info);
}


PYBIND11_MODULE(morphio, m) {

    m.doc() = "pybind11 example plugin"; // optional module docstring


    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const morphio::URI&>())
        .def_property_readonly("points", &morphio::Morphology::points)
        .def_property_readonly("sectionTypes", &morphio::Morphology::sectionTypes)
        .def_property_readonly("section", &morphio::Morphology::section)
        .def_property_readonly("sections", &morphio::Morphology::sections)
        .def_property_readonly("rootSections", &morphio::Morphology::rootSections)
        .def_property_readonly("soma", &morphio::Morphology::soma)
        .def_property_readonly("cellFamily", &morphio::Morphology::cellFamily)
        .def_property_readonly("somaType", &morphio::Morphology::somaType)
        .def_property_readonly("version", &morphio::Morphology::version);

    py::class_<morphio::Soma>(m, "Soma")
        .def(py::init<const morphio::Soma&>())
        .def_property_readonly("somaCenter", &morphio::Soma::somaCenter)
        .def_property_readonly("points", [](morphio::Soma* soma){ return span_array_to_ndarray(soma->points()); })
        .def_property_readonly("diameters", [](morphio::Soma* soma){ return span_to_ndarray(soma->diameters()); });

    py::class_<morphio::Section>(m, "Section")
        .def_property_readonly("parent", &morphio::Section::parent)
        .def_property_readonly("type", &morphio::Section::type)
        .def_property_readonly("isRoot", &morphio::Section::isRoot)
        .def_property_readonly("parent", &morphio::Section::parent)
        .def_property_readonly("children", &morphio::Section::children)
        .def_property_readonly("points", [](morphio::Section* section){ return span_array_to_ndarray(section->points()); })
        .def_property_readonly("diameters", [](morphio::Section* section){ return span_to_ndarray(section->diameters()); })
        .def_property_readonly("perimeters", [](morphio::Section* section){ return span_to_ndarray(section->perimeters()); })
        .def_property_readonly("id", &morphio::Section::id);

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
    py::class_<morphio::mut::Morphology>(mut_module, "Morphology")
        .def(py::init<>())
        .def_property_readonly("sections", &morphio::mut::Morphology::sections)
        .def_property_readonly("root_sections", &morphio::mut::Morphology::rootSections)
        .def_property_readonly("soma", &morphio::mut::Morphology::soma)
        .def("parent", &morphio::mut::Morphology::parent)
        .def("children", &morphio::mut::Morphology::children)
        .def("section", &morphio::mut::Morphology::section)
        .def("build_read_only", &morphio::mut::Morphology::buildReadOnly)
        .def("deleteSection", &morphio::mut::Morphology::deleteSection)
        .def("appendSection", (uint32_t (morphio::mut::Morphology::*) (int32_t, morphio::SectionType, const morphio::Property::PointLevel&)) &morphio::mut::Morphology::appendSection)
        .def("deleteSection", &morphio::mut::Morphology::deleteSection)
        .def("traverse", &morphio::mut::Morphology::traverse);


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
        .def_property("points",
                      &morphio::mut::Soma::points,
                      [](morphio::mut::Soma* soma,
                         const std::vector<morphio::Point>& _points) {
                          soma -> points() = _points;
                      })
        .def_property("diameters",
                      &morphio::mut::Soma::diameters,
                      [](morphio::mut::Soma* soma,
                         const std::vector<float>& _diameters) {
                          soma -> diameters() = _diameters;
                      });



    py::class_<morphio::Property::PointLevel>(m, "PointLevel")
        .def(py::init<>())
        .def(py::init<std::vector<morphio::Property::Point::Type>,
                       std::vector<morphio::Property::Diameter::Type>,
                       std::vector<morphio::Property::Perimeter::Type>>())
        .def_readwrite("points", &morphio::Property::PointLevel::_points)
        .def_readwrite("perimeters", &morphio::Property::PointLevel::_perimeters)
        .def_readwrite("diameters", &morphio::Property::PointLevel::_diameters);
}
