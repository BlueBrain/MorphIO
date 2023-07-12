/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bind_enums.h"
#include "../../include/morphio/enums.h"

namespace py = pybind11;

void bind_enums(py::module& m) {
    py::enum_<morphio::enums::AnnotationType>(m, "AnnotationType", py::arithmetic())
        .value("single_child",
               morphio::enums::AnnotationType::SINGLE_CHILD,
               "Indicates that a section has only one child");

    py::enum_<IterType>(m, "IterType", py::arithmetic())
        .value("depth_first", IterType::DEPTH_FIRST)
        .value("breadth_first", IterType::BREADTH_FIRST)
        .value("upstream", IterType::UPSTREAM)
        .export_values();

    py::enum_<morphio::enums::LogLevel>(m, "LogLevel")
        .value("error", morphio::enums::LogLevel::ERROR)
        .value("warning", morphio::enums::LogLevel::WARNING)
        .value("info", morphio::enums::LogLevel::INFO)
        .value("debug", morphio::enums::LogLevel::DEBUG);

    py::enum_<morphio::enums::SectionType>(m, "SectionType", py::arithmetic())
        .value("undefined", morphio::enums::SectionType::SECTION_UNDEFINED)
        .value("soma", morphio::enums::SectionType::SECTION_SOMA)
        .value("axon", morphio::enums::SectionType::SECTION_AXON)
        .value("basal_dendrite", morphio::enums::SectionType::SECTION_DENDRITE)
        .value("apical_dendrite", morphio::enums::SectionType::SECTION_APICAL_DENDRITE)
        .value("custom5", morphio::enums::SectionType::SECTION_CUSTOM_5)
        .value("custom6", morphio::enums::SectionType::SECTION_CUSTOM_6)
        .value("custom7", morphio::enums::SectionType::SECTION_CUSTOM_7)
        .value("custom8", morphio::enums::SectionType::SECTION_CUSTOM_8)
        .value("custom9", morphio::enums::SectionType::SECTION_CUSTOM_9)
        .value("custom10", morphio::enums::SectionType::SECTION_CUSTOM_10)
        .value("custom11", morphio::enums::SectionType::SECTION_CUSTOM_11)
        .value("custom12", morphio::enums::SectionType::SECTION_CUSTOM_12)
        .value("custom13", morphio::enums::SectionType::SECTION_CUSTOM_13)
        .value("custom14", morphio::enums::SectionType::SECTION_CUSTOM_14)
        .value("custom15", morphio::enums::SectionType::SECTION_CUSTOM_15)
        .value("custom16", morphio::enums::SectionType::SECTION_CUSTOM_16)
        .value("custom17", morphio::enums::SectionType::SECTION_CUSTOM_17)
        .value("custom18", morphio::enums::SectionType::SECTION_CUSTOM_18)
        .value("custom19", morphio::enums::SectionType::SECTION_CUSTOM_19)
        .value("glia_perivascular_process",
               morphio::enums::SectionType::SECTION_GLIA_PERIVASCULAR_PROCESS)
        .value("glia_process", morphio::enums::SectionType::SECTION_GLIA_PROCESS)
        .value("spine_head", morphio::enums::SectionType::SECTION_SPINE_HEAD)
        .value("spine_neck", morphio::enums::SectionType::SECTION_SPINE_NECK)
        .value("all", morphio::enums::SectionType::SECTION_ALL)
        .export_values();

    py::enum_<morphio::enums::VascularSectionType>(m, "VasculatureSectionType", py::arithmetic())
        .value("undefined", morphio::enums::VascularSectionType::SECTION_NOT_DEFINED)
        .value("vein", morphio::enums::VascularSectionType::SECTION_VEIN)
        .value("artery", morphio::enums::VascularSectionType::SECTION_ARTERY)
        .value("venule", morphio::enums::VascularSectionType::SECTION_VENULE)
        .value("arteriole", morphio::enums::VascularSectionType::SECTION_ARTERIOLE)
        .value("venous_capillary", morphio::enums::VascularSectionType::SECTION_VENOUS_CAPILLARY)
        .value("arterial_capillary",
               morphio::enums::VascularSectionType::SECTION_ARTERIAL_CAPILLARY)
        .value("transitional", morphio::enums::VascularSectionType::SECTION_TRANSITIONAL)
        .export_values();

    py::enum_<morphio::enums::Option>(m, "Option", py::arithmetic())
        .value("no_modifier", morphio::enums::Option::NO_MODIFIER)
        .value("two_points_sections", morphio::enums::Option::TWO_POINTS_SECTIONS)
        .value("soma_sphere", morphio::enums::Option::SOMA_SPHERE)
        .value("no_duplicates", morphio::enums::Option::NO_DUPLICATES)
        .value("nrn_order", morphio::enums::Option::NRN_ORDER)
        .export_values();


    py::enum_<morphio::enums::CellFamily>(m, "CellFamily", py::arithmetic())
        .value("NEURON", morphio::enums::CellFamily::NEURON)
        .value("GLIA", morphio::enums::CellFamily::GLIA)
        .value("SPINE", morphio::enums::CellFamily::SPINE)
        .export_values();


    py::enum_<morphio::enums::Warning>(m, "Warning")
        .value("undefined", morphio::enums::Warning::UNDEFINED)
        .value("mitochondria_write_not_supported",
               morphio::enums::Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED)
        .value("write_no_soma", morphio::enums::Warning::WRITE_NO_SOMA)
        .value("soma_non_conform", morphio::enums::SOMA_NON_CONFORM)
        .value("no_soma_found", morphio::enums::Warning::NO_SOMA_FOUND)
        .value("disconnected_neurite", morphio::enums::DISCONNECTED_NEURITE)
        .value("wrong_duplicate", morphio::enums::WRONG_DUPLICATE)
        .value("appending_empty_section", morphio::enums::APPENDING_EMPTY_SECTION)
        .value("wrong_root_point", morphio::enums::Warning::WRONG_ROOT_POINT)
        .value("only_child", morphio::enums::Warning::ONLY_CHILD)
        .value("write_empty_morphology", morphio::enums::WRITE_EMPTY_MORPHOLOGY)
        .value("zero_diameter", morphio::enums::Warning::ZERO_DIAMETER)
        .value("soma_non_contour", morphio::enums::Warning::SOMA_NON_CONTOUR)
        .value("soma_non_cylinder_or_point", morphio::enums::Warning::SOMA_NON_CYLINDER_OR_POINT);

    py::enum_<morphio::enums::SomaType>(m, "SomaType", py::arithmetic())
        .value("SOMA_UNDEFINED", morphio::enums::SomaType::SOMA_UNDEFINED)
        .value("SOMA_SINGLE_POINT", morphio::enums::SomaType::SOMA_SINGLE_POINT)
        .value("SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS",
               morphio::enums::SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
        .value("SOMA_CYLINDERS", morphio::enums::SomaType::SOMA_CYLINDERS)
        .value("SOMA_SIMPLE_CONTOUR", morphio::enums::SomaType::SOMA_SIMPLE_CONTOUR);
}
