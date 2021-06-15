#include "bind_misc.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/enums.h>
#include <morphio/errorMessages.h>
#include <morphio/types.h>
#include <morphio/version.h>

#include "../../include/morphio/enums.h"
#include "bind_enums.h"

namespace py = pybind11;

void bind_misc(py::module& m) {
    using namespace py::literals;

    m.def("set_maximum_warnings",
          &morphio::set_maximum_warnings,
          "Set the maximum number of warnings to be printed on screen\n"
          "0 will print no warning\n"
          "-1 will print them all");
    m.def("set_raise_warnings", &morphio::set_raise_warnings, "Whether to raise warning as errors");
    m.def("set_ignored_warning",
          static_cast<void (*)(morphio::Warning, bool)>(&morphio::set_ignored_warning),
          "Ignore/Unignore a specific warning message",
          "warning"_a,
          "ignore"_a = true);
    m.def("set_ignored_warning",
          static_cast<void (*)(const std::vector<morphio::Warning>&, bool)>(
              &morphio::set_ignored_warning),
          "Ignore/Unignore a list of warnings",
          "warning"_a,
          "ignore"_a = true);

    py::enum_<morphio::enums::AnnotationType>(m, "AnnotationType")
        .value("single_child",
               morphio::enums::AnnotationType::SINGLE_CHILD,
               "Indicates that a section has only one child");

    py::enum_<IterType>(m, "IterType")
        .value("depth_first", IterType::DEPTH_FIRST)
        .value("breadth_first", IterType::BREADTH_FIRST)
        .value("upstream", IterType::UPSTREAM)
        .export_values();

    py::enum_<morphio::enums::LogLevel>(m, "LogLevel")
        .value("error", morphio::enums::LogLevel::ERROR)
        .value("warning", morphio::enums::LogLevel::WARNING)
        .value("info", morphio::enums::LogLevel::INFO)
        .value("debug", morphio::enums::LogLevel::DEBUG);

    py::enum_<morphio::enums::SectionType>(m, "SectionType")
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
        .value("glia_perivascular_process",
               morphio::enums::SectionType::SECTION_GLIA_PERIVASCULAR_PROCESS)
        .value("glia_process", morphio::enums::SectionType::SECTION_GLIA_PROCESS)
        .value("all", morphio::enums::SectionType::SECTION_ALL)
        .export_values();

    py::enum_<morphio::enums::VascularSectionType>(m, "VasculatureSectionType")
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


    py::enum_<morphio::enums::CellFamily>(m, "CellFamily")
        .value("NEURON", morphio::enums::CellFamily::NEURON)
        .value("GLIA", morphio::enums::CellFamily::GLIA)
        .export_values();


    py::enum_<morphio::enums::Warning>(m, "Warning")
        .value("undefined", morphio::enums::Warning::UNDEFINED)
        .value("mitochondria_write_not_supported",
               morphio::enums::Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED)
        .value("write_no_soma", morphio::enums::Warning::WRITE_NO_SOMA)
        .value("write_empty_morphology", morphio::enums::Warning::WRITE_EMPTY_MORPHOLOGY)
        .value("soma_non_conform", morphio::enums::SOMA_NON_CONFORM)
        .value("no_soma_found", morphio::enums::Warning::NO_SOMA_FOUND)
        .value("disconnected_neurite", morphio::enums::DISCONNECTED_NEURITE)
        .value("wrong_duplicate", morphio::enums::WRONG_DUPLICATE)
        .value("appending_empty_section", morphio::enums::APPENDING_EMPTY_SECTION)
        .value("wrong_root_point", morphio::enums::Warning::WRONG_ROOT_POINT)
        .value("only_child", morphio::enums::Warning::ONLY_CHILD)
        .value("zero_diameter", morphio::enums::Warning::ZERO_DIAMETER);

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
        .value("SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS",
               morphio::enums::SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS)
        .value("SOMA_CYLINDERS", morphio::enums::SomaType::SOMA_CYLINDERS)
        .value("SOMA_SIMPLE_CONTOUR", morphio::enums::SomaType::SOMA_SIMPLE_CONTOUR);

    m.attr("version") = morphio::getVersionString();

    auto base = py::register_exception<morphio::MorphioError&>(m, "MorphioError");
    // base.ptr() signifies "inherits from"
    auto raw = py::register_exception<morphio::RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<morphio::UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<morphio::SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<morphio::IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<morphio::MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<morphio::MissingParentError&>(m, "MissingParentError", raw.ptr());
    py::register_exception<morphio::SectionBuilderError&>(m, "SectionBuilderError", raw.ptr());
    py::register_exception<morphio::WriterError&>(m, "WriterError", base.ptr());


    py::class_<morphio::Points>(m, "Points", py::buffer_protocol())
        .def_buffer([](morphio::Points& points) -> py::buffer_info {
            return py::buffer_info(points.data(),              /* Pointer to buffer */
                                   sizeof(morphio::floatType), /* Size of one scalar */
                                   py::format_descriptor<morphio::floatType>::format(), /* Python
                                                                              struct-style format
                                                                              descriptor */
                                   2, /* Number of dimensions */
                                   {static_cast<ssize_t>(points.size()),
                                    static_cast<ssize_t>(3)},       /* Buffer dimensions */
                                   {sizeof(morphio::floatType) * 3, /* Strides (in bytes) for each
                                                                       index */
                                    sizeof(morphio::floatType)});
        });

    py::class_<morphio::Property::Properties>(
        m, "Properties", "The higher level container structure is Property::Properties")
        .def_readwrite("point_level",
                       &morphio::Property::Properties::_pointLevel,
                       "Returns the structure that stores information at the point level")
        .def_readwrite("section_level",
                       &morphio::Property::Properties::_sectionLevel,
                       "Returns the structure that stores information at the section level")
        .def_readwrite("cell_level",
                       &morphio::Property::Properties::_cellLevel,
                       "Returns the structure that stores information at the cell level");


    py::class_<morphio::Property::PointLevel>(m,
                                              "PointLevel",
                                              "Container class for information available at the "
                                              "point level (point coordinate, diameter, perimeter)")
        .def(py::init<>())
        .def(py::init<std::vector<morphio::Property::Point::Type>,
                      std::vector<morphio::Property::Diameter::Type>>(),
             "points"_a,
             "diameters"_a)
        .def(py::init<std::vector<morphio::Property::Point::Type>,
                      std::vector<morphio::Property::Diameter::Type>,
                      std::vector<morphio::Property::Perimeter::Type>>(),
             "points"_a,
             "diameters"_a,
             "perimeters"_a)
        .def_readwrite("points",
                       &morphio::Property::PointLevel::_points,
                       "Returns the list of point coordinates")
        .def_readwrite("perimeters",
                       &morphio::Property::PointLevel::_perimeters,
                       "Returns the list of perimeters")
        .def_readwrite("diameters",
                       &morphio::Property::PointLevel::_diameters,
                       "Returns the list of diameters");

    py::class_<morphio::Property::SectionLevel>(m,
                                                "SectionLevel",
                                                "Container class for information available at the "
                                                "section level (section type, parent section)")
        .def_readwrite("sections",
                       &morphio::Property::SectionLevel::_sections,
                       "Returns a list of [offset, parent section ID]")
        .def_readwrite("section_types",
                       &morphio::Property::SectionLevel::_sectionTypes,
                       "Returns the list of section types")
        .def_readwrite("children",
                       &morphio::Property::SectionLevel::_children,
                       "Returns a dictionary where key is a section ID "
                       "and value is the list of children section IDs");

    py::class_<morphio::Property::CellLevel>(m,
                                             "CellLevel",
                                             "Container class for information available at the "
                                             "cell level (cell type, file version, soma type)")
        .def_readwrite("cell_family",
                       &morphio::Property::CellLevel::_cellFamily,
                       "Returns the cell family (neuron or glia)")
        .def_readwrite("soma_type",
                       &morphio::Property::CellLevel::_somaType,
                       "Returns the soma type")
        .def_readwrite("version", &morphio::Property::CellLevel::_version, "Returns the version");

    py::class_<morphio::Property::Annotation>(
        m,
        "Annotation",
        "Container class for information about anomalies detected while parsing the file (no soma, "
        "section with a single child...)")
        .def_readwrite("type", &morphio::Property::Annotation::_type, "Returns the type")
        .def_readwrite("section_id",
                       &morphio::Property::Annotation::_sectionId,
                       "Returns the sectionId")
        .def_readwrite("line_number",
                       &morphio::Property::Annotation::_lineNumber,
                       "Returns the lineNumber")
        .def_readwrite("details", &morphio::Property::Annotation::_details, "Returns the details")
        .def_property_readonly(
            "points",
            [](morphio::Property::Annotation* a) { return a->_points._points; },
            "Returns the list of coordinates of annotated points")
        .def_property_readonly(
            "diameters",
            [](morphio::Property::Annotation* a) { return a->_points._diameters; },
            "Returns the list of diameters of annotated points")
        .def_property_readonly(
            "perimeters",
            [](morphio::Property::Annotation* a) { return a->_points._perimeters; },
            "Returns the list of perimeters of annotated points");

    py::class_<morphio::Property::Marker>(m,
                                          "Marker",
                                          "Container class for NeuroLucida extra Markers ")
        .def_property_readonly(
            "label",
            [](morphio::Property::Marker* marker) { return marker->_label; },
            "Returns the label")
        .def_property_readonly(
            "points",
            [](morphio::Property::Marker* marker) { return marker->_pointLevel._points; },
            "Returns the list of coordinates of the marker points")
        .def_property_readonly(
            "diameters",
            [](morphio::Property::Marker* marker) { return marker->_pointLevel._diameters; },
            "Returns the list of diameters of the marker points")
        .def_property_readonly(
            "section_id",
            [](morphio::Property::Marker* marker) { return marker->_sectionId; },
            "Returns the id of section that contains the marker");

    py::class_<morphio::Property::MitochondriaPointLevel>(
        m,
        "MitochondriaPointLevel",
        "Container class for the information available at the mitochondrial point level (enclosing "
        "neuronal section, relative distance to start of neuronal section, diameter)")
        .def(py::init<>())
        .def(py::init<std::vector<uint32_t>,
                      std::vector<morphio::floatType>,
                      std::vector<morphio::Property::Diameter::Type>>(),
             "neuronal_section_ids"_a,
             "distances_to_section_start"_a,
             "diameters"_a);
}
