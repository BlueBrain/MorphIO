#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <morphio/types.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/enums.h>


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



PYBIND11_MODULE(python_morphio, m) {

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
        .def("points", &morphio::Soma::points)
        .def("diameters", &morphio::Soma::diameters)
        .def("type", &morphio::Soma::type);

    py::class_<morphio::Section>(m, "Section")
        .def("parent", &morphio::Section::parent)
        .def("type", &morphio::Section::type)
        .def("isRoot", &morphio::Section::isRoot)
        .def("parent", &morphio::Section::parent)
        .def("points", &morphio::Section::points)
        .def("diameters", &morphio::Section::diameters)
        .def("perimeters", &morphio::Section::perimeters)
        .def("id", &morphio::Section::id);

    py::enum_<morphio::enums::SectionType>(m, "SectionType")
        .value("SECTION_UNDEFINED", morphio::enums::SectionType::SECTION_UNDEFINED)
        .value("SECTION_SOMA", morphio::enums::SectionType::SECTION_SOMA)
        .value("SECTION_AXON", morphio::enums::SectionType::SECTION_AXON)
        .value("SECTION_DENDRITE", morphio::enums::SectionType::SECTION_DENDRITE)
        .value("SECTION_APICAL_DENDRITE", morphio::enums::SectionType::SECTION_APICAL_DENDRITE)
        .value("SECTION_GLIA_PROCESS", morphio::enums::SectionType::SECTION_GLIA_PROCESS)
        .value("SECTION_GLIA_ENDFOOT", morphio::enums::SectionType::SECTION_GLIA_ENDFOOT)
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
