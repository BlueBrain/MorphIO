#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <brion/enums.h>
#include <brion/morphology.h>

namespace py = pybind11;

/** Loading/Casting of vmmlib::vector into numpy arrays **/
namespace pybind11 {
namespace detail {
template <size_t M, typename T> struct type_caster<vmml::vector<M,T>>
{
public:
    typedef vmml::vector<M,T> type;
    PYBIND11_TYPE_CASTER(type, _("vmml::vector<M,T>"));


    // Conversion part 1 (Python -> C++)
    bool load(py::handle src, bool convert)
        {
            if (!convert && !py::array_t<T>::check_(src))
                return false;

            auto buf = py::array_t<T, py::array::c_style | py::array::forcecast>::ensure(src);
            if (!buf)
                return false;

            value = vmml::vector<M,T>(buf.data());

            return true;
        }

    //Conversion part 2 (C++ -> Python)
    static py::handle cast(const vmml::vector<M,T>& src, py::return_value_policy policy, py::handle parent)
        {
            py::array a(M, src.array);
            return a.release();
        }
};


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
