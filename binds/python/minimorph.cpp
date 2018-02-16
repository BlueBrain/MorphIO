#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <minimorph/types.h>
#include <minimorph/morphology.h>
#include <minimorph/section.h>
#include <minimorph/soma.h>
#include <minimorph/enums.h>


namespace py = pybind11;

/** Loading/Casting of vmmlib::vector into numpy arrays **/
namespace pybind11 {
namespace detail {
// template <size_t M, typename T> struct type_caster<vmml::vector<M,T>>
// {
// public:
//     typedef vmml::vector<M,T> type;
//     PYBIND11_TYPE_CASTER(type, _("vmml::vector<M,T>"));


//     // Conversion part 1 (Python -> C++)
//     bool load(py::handle src, bool convert)
//         {
//             if (!convert && !py::array_t<T>::check_(src))
//                 return false;

//             auto buf = py::array_t<T, py::array::c_style | py::array::forcecast>::ensure(src);
//             if (!buf)
//                 return false;

//             value = vmml::vector<M,T>(buf.data());

//             return true;
//         }

//     //Conversion part 2 (C++ -> Python)
//     static py::handle cast(const vmml::vector<M,T>& src, py::return_value_policy policy, py::handle parent)
//         {
//             py::array a(M, src.array);
//             return a.release();
//         }
// };


// template <size_t M, typename T> struct type_caster<std::vector<vmml::vector<M,T>>>
// {
// public:
//     typedef std::vector<vmml::vector<M,T>> type;
//     PYBIND11_TYPE_CASTER(type, _("std::vector<vmml::vector<M,T>>"));

//     // Conversion part 1 (Python -> C++)
//     bool load(py::handle src, bool convert)
//         {
//             if (!convert && !py::array_t<T>::check_(src))
//                 return false;

//             auto buf = py::array_t<T, py::array::c_style | py::array::forcecast>::ensure(src);
//             if (!buf)
//                 return false;

//             auto dims = buf.ndim();
//             if (dims != 2 )
//                 return false;

//             std::vector<size_t> shape(2);
//             value = vmml::vector<M,T>(buf.data());

//             return true;
//         }

//     //Conversion part 2 (C++ -> Python)
//     static py::handle cast(const vmml::vector<M,T>& src, py::return_value_policy policy, py::handle parent)
//         {
//             py::array a(M, src.array);
//             return a.release();
//         }
// };
}} // namespace pybind11::detail



PYBIND11_MODULE(python_minimorph, m) {

    m.doc() = "pybind11 example plugin"; // optional module docstring

    py::class_<gsl::span<std::array<float, 3>>>(m, "spans", py::buffer_protocol())
        .def_buffer([](gsl::span<std::array<float, 3>> &span) -> py::buffer_info {
                return py::buffer_info(
                    span.data(),                            /* Pointer to buffer */
                    sizeof(float),                          /* Size of one scalar */
                    py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                    2,                                      /* Number of dimensions */

                    // Forced cast to prevent error:
                    // template argument deduction/substitution failed */
                    { (int) span.size(), 3 }, /* buffer dimentions */
                    { sizeof(float) * 3,                  /* Strides (in bytes) for each index */
                            sizeof(float) }
                    );
            });


    py::class_<minimorph::Morphology>(m, "Morphology")
        .def(py::init<const minimorph::URI&>())
        .def("points", &minimorph::Morphology::points)
        .def("sectionTypes", &minimorph::Morphology::sectionTypes)
        .def("section", &minimorph::Morphology::section)
        .def("sections", &minimorph::Morphology::sections)
        .def("rootSections", &minimorph::Morphology::rootSections)
        .def("soma", &minimorph::Morphology::soma)
        .def("cellFamily", &minimorph::Morphology::cellFamily)
        .def("version", &minimorph::Morphology::version);

    py::class_<minimorph::Soma>(m, "Soma")
        .def(py::init<const minimorph::Soma&>())
        .def("somaCenter", &minimorph::Soma::somaCenter)
        .def("points", &minimorph::Soma::points)
        .def("diameters", &minimorph::Soma::diameters)
        .def("type", &minimorph::Soma::type);

    py::class_<minimorph::Section>(m, "Section")
        .def("parent", &minimorph::Section::parent)
        .def("type", &minimorph::Section::type)
        .def("isRoot", &minimorph::Section::isRoot)
        .def("parent", &minimorph::Section::parent)
        .def("points", &minimorph::Section::points)
        .def("diameters", &minimorph::Section::diameters)
        .def("perimeters", &minimorph::Section::perimeters)
        .def("id", &minimorph::Section::id);

    py::enum_<minimorph::enums::SectionType>(m, "SectionType")
        .value("SECTION_UNDEFINED", minimorph::enums::SectionType::SECTION_UNDEFINED)
        .value("SECTION_SOMA", minimorph::enums::SectionType::SECTION_SOMA)
        .value("SECTION_AXON", minimorph::enums::SectionType::SECTION_AXON)
        .value("SECTION_DENDRITE", minimorph::enums::SectionType::SECTION_DENDRITE)
        .value("SECTION_APICAL_DENDRITE", minimorph::enums::SectionType::SECTION_APICAL_DENDRITE)
        .value("SECTION_GLIA_PROCESS", minimorph::enums::SectionType::SECTION_GLIA_PROCESS)
        .value("SECTION_GLIA_ENDFOOT", minimorph::enums::SectionType::SECTION_GLIA_ENDFOOT)
        .export_values();


    py::enum_<minimorph::enums::MorphologyVersion>(m, "MorphologyVersion")
        .value("MORPHOLOGY_VERSION_H5_1", minimorph::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1)
        .value("MORPHOLOGY_VERSION_H5_2", minimorph::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_2)
        .value("MORPHOLOGY_VERSION_H5_1_1", minimorph::enums::MorphologyVersion::MORPHOLOGY_VERSION_H5_1_1)
        .value("MORPHOLOGY_VERSION_SWC_1", minimorph::enums::MorphologyVersion::MORPHOLOGY_VERSION_SWC_1)
        .value("MORPHOLOGY_VERSION_UNDEFINED", minimorph::enums::MorphologyVersion::MORPHOLOGY_VERSION_UNDEFINED)
        .export_values();

    py::enum_<minimorph::enums::CellFamily>(m, "CellFamily")
        .value("FAMILY_NEURON", minimorph::enums::CellFamily::FAMILY_NEURON)
        .value("FAMILY_GLIA", minimorph::enums::CellFamily::FAMILY_GLIA)
        .export_values();


    py::enum_<minimorph::enums::AccessMode>(m, "AccessMode")
        .value("MODE_READ", minimorph::enums::AccessMode::MODE_READ)
        .value("MODE_WRITE", minimorph::enums::AccessMode::MODE_WRITE)
        .value("MODE_OVERWRITE", minimorph::enums::AccessMode::MODE_OVERWRITE)
        .value("MODE_READWRITE", minimorph::enums::AccessMode::MODE_READWRITE)
        .value("MODE_READOVERWRITE", minimorph::enums::AccessMode::MODE_READOVERWRITE)
        .export_values();

    auto base = py::register_exception<minimorph::MinimorphError&>(m, "Error");
    auto raw = py::register_exception<minimorph::RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<minimorph::UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<minimorph::SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<minimorph::IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<minimorph::MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<minimorph::MissingParentError&>(m, "MissingParentError", raw.ptr());
}
