#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <morphio/types.h>
#include <morphio/enums.h>

namespace py = pybind11;
using namespace py::literals;

void bind_misc(py::module &m) {

  m.def("set_maximum_warnings", &morphio::set_maximum_warnings,
        "Set the maximum number of warnings to be printed on screen\n"
        "0 will print no warning\n"
        "-1 will print them all");
  m.def("set_ignored_warning", (void (*)(morphio::Warning, bool)) &morphio::set_ignored_warning,
        "Ignore/Unignore a specific warning message", "warning"_a, "ignore"_a = true);
  m.def("set_ignored_warning", (void (*)(const std::vector<morphio::Warning>&, bool)) &morphio::set_ignored_warning,
        "Ignore/Unignore a list of warnings", "warning"_a, "ignore"_a = true);
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

    py::enum_<morphio::enums::Option>(m, "Option", py::arithmetic())
        .value("no_modifier", morphio::enums::Option::NO_MODIFIER)
        .value("two_points_sections", morphio::enums::Option::TWO_POINTS_SECTIONS)
        .value("soma_sphere", morphio::enums::Option::SOMA_SPHERE)
        .value("no_duplicates", morphio::enums::Option::NO_DUPLICATES)
        .value("nrn_order", morphio::enums::Option::NRN_ORDER)
        .export_values();


    py::enum_<morphio::enums::MorphologyVersion>(m, "MorphologyVersion")
        .value("MORPHOLOGY_VERSION_H5_1",
               morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1)
        .value("MORPHOLOGY_VERSION_H5_2",
               morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_2)
        .value("MORPHOLOGY_VERSION_H5_1_1",
               morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1_1)
        .value("MORPHOLOGY_VERSION_SWC_1",
               morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_SWC_1)
        .value("MORPHOLOGY_VERSION_UNDEFINED",
               morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_UNDEFINED)
        .value("MORPHOLOGY_VERSION_ASC_1",
               morphio::enums::MorphologyVersion::MORPHOLOGY_VERSION_ASC_1)
        .export_values();

    py::enum_<morphio::enums::CellFamily>(m, "CellFamily")
        .value("FAMILY_NEURON", morphio::enums::CellFamily::FAMILY_NEURON)
        .value("FAMILY_GLIA", morphio::enums::CellFamily::FAMILY_GLIA)
        .export_values();


    py::enum_<morphio::enums::Warning>(m, "Warning")
        .value("undefined", morphio::enums::Warning::UNDEFINED)
        .value("mitochondria_write_not_supported", morphio::enums::Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED)
        .value("write_no_soma", morphio::enums::Warning::WRITE_NO_SOMA)
        .value("no_soma_found", morphio::enums::Warning::NO_SOMA_FOUND)
        .value("wrong_root_point", morphio::enums::Warning::WRONG_ROOT_POINT)
        .value("only_child", morphio::enums::Warning::ONLY_CHILD);

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
        .value("SOMA_SIMPLE_CONTOUR", morphio::enums::SomaType::SOMA_SIMPLE_CONTOUR);

    m.attr("version") = morphio::VERSION;

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

    m.doc() = "pybind11 example plugin"; // optional module docstring
}
