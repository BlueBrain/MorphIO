#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <morphio/types.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/enums.h>


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


PYBIND11_MODULE(python_morphio, m) {

    m.doc() = "pybind11 example plugin"; // optional module docstring

    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const morphio::URI&>())
        .def("points", &morphio::Morphology::points)
        .def("sectionTypes", &morphio::Morphology::sectionTypes)
        .def("section", &morphio::Morphology::section)
        .def("sections", &morphio::Morphology::sections)
        .def("rootSections", &morphio::Morphology::rootSections)
        .def("soma", &morphio::Morphology::soma)
        .def("cellFamily", &morphio::Morphology::cellFamily)
        .def("version", &morphio::Morphology::version);

    py::class_<morphio::Soma>(m, "Soma")
        .def(py::init<const morphio::Soma&>())
        .def("somaCenter", &morphio::Soma::somaCenter)
        .def("points", [](morphio::Soma* soma){ return span_array_to_ndarray(soma->points()); })
        .def("diameters", [](morphio::Soma* soma){ return span_to_ndarray(soma->diameters()); });

    py::class_<morphio::Section>(m, "Section")
        .def("parent", &morphio::Section::parent)
        .def("type", &morphio::Section::type)
        .def("isRoot", &morphio::Section::isRoot)
        .def("parent", &morphio::Section::parent)
        .def("children", &morphio::Section::children)
        .def("points", [](morphio::Section* section){ return span_array_to_ndarray(section->points()); })
        .def("diameters", [](morphio::Section* section){ return span_to_ndarray(section->diameters()); })
        .def("perimeters", [](morphio::Section* section){ return span_to_ndarray(section->perimeters()); })
        .def("id", &morphio::Section::id);

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

    auto base = py::register_exception<morphio::MorphioError&>(m, "Error");
    auto raw = py::register_exception<morphio::RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<morphio::UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<morphio::SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<morphio::IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<morphio::MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<morphio::MissingParentError&>(m, "MissingParentError", raw.ptr());
}
