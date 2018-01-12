#include <pybind11/pybind11.h>
#include <brion/enums.h>
#include <brion/morphology.h>


namespace py = pybind11;


PYBIND11_MODULE(python_brion, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring


    py::class_<brion::Morphology>(m, "Morphology")
        .def(py::init<const brion::URI&>())
        .def("getCellFamily", &brion::Morphology::getCellFamily)
        .def("getPoints", (brion::Vector4fs& (brion::Morphology::*)())&brion::Morphology::getPoints)
        .def("getSections", (brion::Vector2is& (brion::Morphology::*)())&brion::Morphology::getSections)
        .def("getSectionTypes", (brion::SectionTypes& (brion::Morphology::*)())&brion::Morphology::getSectionTypes)
        .def("getPerimeters", (brion::floats& (brion::Morphology::*)())&brion::Morphology::getPerimeters)
        .def("getVersion", &brion::Morphology::getVersion);

    py::enum_<brion::enums::SectionType>(m, "SectionType")
        .value("SECTION_UNDEFINED", brion::enums::SectionType::SECTION_UNDEFINED)
        .value("SECTION_SOMA", brion::enums::SectionType::SECTION_SOMA)
        .value("SECTION_AXON", brion::enums::SectionType::SECTION_AXON)
        .value("SECTION_DENDRITE", brion::enums::SectionType::SECTION_DENDRITE)
        .value("SECTION_APICAL_DENDRITE", brion::enums::SectionType::SECTION_APICAL_DENDRITE)
        .value("SECTION_GLIA_PROCESS", brion::enums::SectionType::SECTION_GLIA_PROCESS)
        .value("SECTION_GLIA_ENDFOOT", brion::enums::SectionType::SECTION_GLIA_ENDFOOT)
        .export_values();


    py::enum_<brion::enums::MorphologyVersion>(m, "MorphologyVersion")
        .value("MORPHOLOGY_VERSION_H5_1", brion::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1)
        .value("MORPHOLOGY_VERSION_H5_2", brion::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_2)
        .value("MORPHOLOGY_VERSION_H5_1_1", brion::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1_1)
        .value("MORPHOLOGY_VERSION_SWC_1", brion::enums::MorphologyVersion::MORPHOLOGY_VERSION_SWC_1)
        .value("MORPHOLOGY_VERSION_UNDEFINED", brion::enums::MorphologyVersion::MORPHOLOGY_VERSION_UNDEFINED)
        .export_values();

    py::enum_<brion::enums::CellFamily>(m, "CellFamily")
        .value("FAMILY_NEURON", brion::enums::CellFamily::FAMILY_NEURON)
        .value("FAMILY_GLIA", brion::enums::CellFamily::FAMILY_GLIA)
        .export_values();


    py::enum_<brion::enums::AccessMode>(m, "AccessMode")
        .value("MODE_READ", brion::enums::AccessMode::MODE_READ)
        .value("MODE_WRITE", brion::enums::AccessMode::MODE_WRITE)
        .value("MODE_OVERWRITE", brion::enums::AccessMode::MODE_OVERWRITE)
        .value("MODE_READWRITE", brion::enums::AccessMode::MODE_READWRITE)
        .value("MODE_READOVERWRITE", brion::enums::AccessMode::MODE_READOVERWRITE)
        .export_values();
}
