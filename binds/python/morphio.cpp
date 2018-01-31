#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <minimorph/enums.h>
#include <minimorph/morphology.h>


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


PYBIND11_MODULE(python_morphio, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring


    py::class_<minimorph::Morphology>(m, "BrionMorphology")
        .def(py::init<const minimorph::URI&>())
        .def("getCellFamily", &minimorph::Morphology::getCellFamily)
        .def("getPoints", (minimorph::Vector4fs& (minimorph::Morphology::*)())&minimorph::Morphology::getPoints)
        .def("getSections", (minimorph::Vector2is& (minimorph::Morphology::*)())&minimorph::Morphology::getSections)
        .def("getSectionTypes", (minimorph::SectionTypes& (minimorph::Morphology::*)())&minimorph::Morphology::getSectionTypes)
        .def("getPerimeters", (minimorph::floats& (minimorph::Morphology::*)())&minimorph::Morphology::getPerimeters)
        .def("getVersion", &minimorph::Morphology::getVersion);

    // .def(py::init<const minimorph::URI&, const minimorph::Matrix4f&>());
    py::class_<morphio::Morphology>(m, "Morphology")
        .def(py::init<const minimorph::URI&>())
        .def("getPoints", (minimorph::Vector4fs& (morphio::Morphology::*)())&morphio::Morphology::getPoints)
        .def("getSectionTypes", (morphio::SectionTypes& (morphio::Morphology::*)())&morphio::Morphology::getSectionTypes)
        .def("getSectionIDs", (morphio::uint32_ts (morphio::Morphology::*)())&morphio::Morphology::getSectionIDs)
        .def("getSections", (const morphio::Vector2is& (morphio::Morphology::*)() const)&morphio::Morphology::getSections)
        .def("getSections", (morphio::Sections (morphio::Morphology::*)(morphio::SectionType) const)&morphio::Morphology::getSections)
        .def("getSections", (morphio::Sections (morphio::Morphology::*)(const morphio::SectionTypes&) const)&morphio::Morphology::getSections)
        .def("getSection", (morphio::Section& (morphio::Morphology::*)(const morphio::uint32_ts&))&morphio::Morphology::getSection)
        .def("getRootSections", &morphio::Morphology::getRootSections)
        .def("getSoma", (morphio::Soma (morphio::Morphology::*)())&morphio::Morphology::getSoma)
        .def("getVersion", &morphio::Morphology::getVersion);
        // .def("getTransformation", (minimorph::Matrix4f& (morphio::Morphology::*)()&morphio::Morphology::getTransformation));

    py::class_<morphio::Soma>(m, "Soma")
        .def(py::init<const morphio::Soma&>())
        .def("getProfilePoints", &morphio::Soma::getProfilePoints)
        .def("getMeanRadius", &morphio::Soma::getMeanRadius)
        .def("getCentroid", &morphio::Soma::getCentroid)
        .def("getChildren", &morphio::Soma::getChildren);

    py::class_<morphio::Section>(m, "Section")
        .def("getID", &morphio::Section::getID)
        .def("getType", &morphio::Section::getType)
        .def("hasParent", &morphio::Section::hasParent)
        .def("getParent", &morphio::Section::getParent)
        .def("getSamples", (morphio::Vector4fs (morphio::Section::*)() const) &morphio::Section::getSamples)
        .def("getChildren", &morphio::Section::getChildren);

    py::enum_<morphio::SectionType>(m, "SectionType")
        .value("soma", morphio::SectionType::soma)
        .value("axon", morphio::SectionType::axon)
        .value("basal_dendrite", morphio::SectionType::dendrite)
        .value("apical_dendrite", morphio::SectionType::apicalDendrite)
        // .value("basal_dendrite", morphio::SectionType::basalDendrite)
        .value("undefined", morphio::SectionType::undefined)
        .value("all", morphio::SectionType::all)
        .export_values();

    py::enum_<minimorph::enums::SectionType>(m, "BrionSectionType")
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

    auto base = py::register_exception<minimorph::Error&>(m, "Error");
    auto raw = py::register_exception<minimorph::RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<minimorph::UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<minimorph::SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<minimorph::IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<minimorph::MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<minimorph::MissingParentError&>(m, "MissingParentError", raw.ptr());
}
