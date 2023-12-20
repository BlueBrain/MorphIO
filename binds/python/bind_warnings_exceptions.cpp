/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "bind_warnings_exceptions.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/collection.h>
#include <morphio/enums.h>
#include <morphio/errorMessages.h>
#include <morphio/types.h>
#include <morphio/version.h>

#include "generated/docstrings.h"


namespace py = pybind11;

void bind_warnings_exceptions(py::module& m) {
    using namespace morphio;
    using namespace py::literals;

    m.def("set_maximum_warnings", &set_maximum_warnings, DOC(morphio, set_maximum_warnings));
    m.def("set_raise_warnings", &set_raise_warnings, DOC(morphio, set_raise_warnings));
    m.def("set_ignored_warning",
          static_cast<void (*)(Warning, bool)>(&set_ignored_warning),
          DOC(morphio, set_ignored_warning),
          "warning"_a,
          "ignore"_a = true);
    m.def("set_ignored_warning",
          static_cast<void (*)(const std::vector<Warning>&, bool)>(&set_ignored_warning),
          DOC(morphio, set_ignored_warning),
          "warning"_a,
          "ignore"_a = true);

    auto base = py::register_exception<MorphioError&>(m, "MorphioError");
    // base.ptr() signifies "inherits from"
    auto raw = py::register_exception<RawDataError&>(m, "RawDataError", base.ptr());
    py::register_exception<UnknownFileType&>(m, "UnknownFileType", base.ptr());
    py::register_exception<SomaError&>(m, "SomaError", base.ptr());
    py::register_exception<IDSequenceError&>(m, "IDSequenceError", raw.ptr());
    py::register_exception<MultipleTrees&>(m, "MultipleTrees", raw.ptr());
    py::register_exception<MissingParentError&>(m, "MissingParentError", raw.ptr());
    py::register_exception<SectionBuilderError&>(m, "SectionBuilderError", raw.ptr());
    py::register_exception<WriterError&>(m, "WriterError", base.ptr());

    py::class_<ErrorAndWarningHandler, std::shared_ptr<ErrorAndWarningHandler>>(
        m, "ErrorAndWarningHandler", "ErrorAndWarningHandler base")
        .def_property_readonly("get_max_warning_count",
                               &ErrorAndWarningHandler::getMaxWarningCount,
                               "ibid");
    py::class_<ErrorAndWarningHandlerCollector,
               ErrorAndWarningHandler,
               std::shared_ptr<ErrorAndWarningHandlerCollector>>(m,
                                                                 "ErrorAndWarningHandlerCollector",
                                                                 "ErrorAndWarningHandler base")
        .def(py::init<>())
        .def("print_all", &ErrorAndWarningHandlerCollector::printAll, "ibid")
        .def("get_all", &ErrorAndWarningHandlerCollector::getAll, "ibid");

    py::class_<ErrorAndWarningHandlerCollector::Emission>(m, "Emission")
        .def_readonly("was_marked_ignore",
                      &ErrorAndWarningHandlerCollector::Emission::wasMarkedIgnore,
                      "ibid")
        .def_readonly("warning", &ErrorAndWarningHandlerCollector::Emission::warning, "ibid");

    py::class_<WarningMessage, std::shared_ptr<WarningMessage>>(m, "WarningMessage")
        .def("warning", &WarningMessage::warning, "ibid")
        .def("msg", &WarningMessage::msg, "ibid")
        .def_readonly("uri", &WarningMessage::uri, "ibid");

#define QUOTE(a) #a
#define C(name) \
    py::class_<name, WarningMessage, std::shared_ptr<name>>(m, QUOTE(name), "WarningMessage")
    C(WarningZeroDiameter).def_readonly("line_number", &WarningZeroDiameter::lineNumber, "ibid");
    C(WarningDisconnectedNeurite)
        .def_readonly("line_number", &WarningDisconnectedNeurite::lineNumber, "ibid");
    (void) C(NoSomaFound);
    C(SomaNonConform).def_readonly("description", &SomaNonConform::description, "ibid");
    C(WrongRootPoint).def_readonly("line_numbers", &WrongRootPoint::lineNumbers, "ibid");
    C(AppendingEmptySection).def_readonly("section_id", &AppendingEmptySection::sectionId, "ibid");
    C(WrongDuplicate)
        .def_readonly("current", &WrongDuplicate::current, "ibid")
        .def_readonly("parent", &WrongDuplicate::parent, "ibid");
    C(OnlyChild)
        .def_readonly("parent_id", &OnlyChild::parentId, "ibid")
        .def_readonly("child_id", &OnlyChild::childId, "ibid");
    (void) C(WriteNoSoma);
    (void) C(WriteEmptyMorphology);
    (void) C(WriteUndefinedSoma);
    (void) C(MitochondriaWriteNotSupported);
    (void) C(SomaNonContour);
    (void) C(SomaNonCynlinderOrPoint);
#undef QUOTE
#undef C
}
