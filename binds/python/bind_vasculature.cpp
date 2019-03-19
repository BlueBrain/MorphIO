#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/iostream.h>
#include <pybind11/operators.h>

#include <morphio/types.h>
#include <morphio/enums.h>
#include <morphio/vasc/morphology.h>
#include <morphio/vasc/section.h>
#include <morphio/vasc/properties.h>

namespace py = pybind11;
using namespace py::literals;

void bind_vasculature(py::module &m) {

    py::class_<morphio::vasculature::VasculatureMorphology>(m, "VasculatureMorphology")
        .def(py::init<const morphio::URI&, unsigned int>(),
             "filename"_a, "options"_a=morphio::enums::Option::NO_MODIFIER)
        // .def(py::init<morphio::mut::Morphology&>())
        .def("__eq__", [](const morphio::vasculature::VasculatureMorphology& a, const morphio::vasculature::VasculatureMorphology& b) {
                return a.operator==(b);
            }, py::is_operator(),
            "Are considered equal, 2 morphologies with the same:\n"
            "- point vector\n"
            "- diameter vector\n"
            "- section types\n"
            "- topology\n")
        .def("__ne__", [](const morphio::vasculature::VasculatureMorphology& a, const morphio::vasculature::VasculatureMorphology& b) {
                return a.operator!=(b);
            }, py::is_operator())
        .def(py::self != py::self)

        // .def("as_mutable", [](const morphio::vasculature::VasculatureMorphology* morph) { return morphio::mut::Morphology(*morph); })

        .def_property_readonly("sections", &morphio::vasculature::VasculatureMorphology::sections,
                               "Returns a vector containing all sections objects\n\n"
                               "Note: To select sections by ID use: VasculatureMorphology::section(id)")

        .def("section", &morphio::vasculature::VasculatureMorphology::section,
             "Returns the Section with the given id\n"
             "throw RawDataError if the id is out of range",
             "section_id"_a)

        // Property accessors
        .def_property_readonly("points", [](morphio::vasculature::VasculatureMorphology* morpho){
                return py::array(morpho->points().size(), morpho->points().data());
            },
            "Returns a list with all points from all sections")
        .def_property_readonly("diameters", [](morphio::vasculature::VasculatureMorphology* morpho){
                auto diameters = morpho->diameters();
                return py::array(diameters.size(), diameters.data());
            },
            "Returns a list with all diameters from all sections")
        .def_property_readonly("section_types", [](morphio::vasculature::VasculatureMorphology* obj){
                auto data = obj->sectionTypes();
                return py::array(data.size(), data.data());
            },
            "Returns a vector with the section type of every section")

        // Iterators
        .def("iter", [](morphio::vasculature::VasculatureMorphology* morpho) {
                return py::make_iterator(morpho->begin(), morpho->end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Iterate on all sections of the graph");


    py::class_<morphio::vasculature::VasculatureSection>(m, "Section")
        // Topology-related member functions
        .def_property_readonly("predecessors", &morphio::vasculature::VasculatureSection::predecessors,
                               "Returns the predecessors section of this section")
        .def_property_readonly("successors", &morphio::vasculature::VasculatureSection::successors,
                               "Returns the successors section of this section")
        .def_property_readonly("neighbors", &morphio::vasculature::VasculatureSection::neighbors,
                               "Returns the neighbors section of this section")

        // Property-related accessors
        .def_property_readonly("id", &morphio::vasculature::VasculatureSection::id,
                               "Returns the section ID\n"
                               "The section ID can be used to query sections via VasculatureSection::section(uint32_t id)")
        .def_property_readonly("type", &morphio::vasculature::VasculatureSection::type,
                               "Returns the morphological type of this section")
        .def_property_readonly("points", [](morphio::vasculature::VasculatureSection* section){ return span_array_to_ndarray(section->points()); },
                               "Returns list of section's point coordinates")
        .def_property_readonly("diameters", [](morphio::vasculature::VasculatureSection* section){ return span_to_ndarray(section->diameters()); },
                               "Returns list of section's point diameters")

        // Iterators
        .def("iter", [](morphio::vasculature::VasculatureSection* section) {
                return py::make_iterator(section->begin(), section->end());
            },
            py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */,
            "Section iterator\n");
}
