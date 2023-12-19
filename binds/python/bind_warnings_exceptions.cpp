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
    using namespace py::literals;

    m.def("set_maximum_warnings",
          &morphio::set_maximum_warnings,
          DOC(morphio, set_maximum_warnings));
    m.def("set_raise_warnings", &morphio::set_raise_warnings, DOC(morphio, set_raise_warnings));
    m.def("set_ignored_warning",
          static_cast<void (*)(morphio::Warning, bool)>(&morphio::set_ignored_warning),
          DOC(morphio, set_ignored_warning),
          "warning"_a,
          "ignore"_a = true);
    m.def("set_ignored_warning",
          static_cast<void (*)(const std::vector<morphio::Warning>&, bool)>(
              &morphio::set_ignored_warning),
          DOC(morphio, set_ignored_warning),
          "warning"_a,
          "ignore"_a = true);

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

    py::class_<morphio::Caution>(m, "Caution")
        .def_readonly("level", &morphio::Caution::level)
        .def_readonly("msg", &morphio::Caution::msg);

    py::class_<morphio::ErrorAndWarningHandler, std::shared_ptr<morphio::ErrorAndWarningHandler>>(
        m, "ErrorAndWarningHandler", "ErrorAndWarningHandler base")
        .def_property_readonly("get_max_warning_count",
                               &morphio::ErrorAndWarningHandler::getMaxWarningCount,
                               "ibid");
    py::class_<morphio::ErrorAndWarningHandlerCollector,
               morphio::ErrorAndWarningHandler,
               std::shared_ptr<morphio::ErrorAndWarningHandlerCollector>>(
        m, "ErrorAndWarningHandlerCollector", "ErrorAndWarningHandler base")
        .def(py::init<>())
        .def("print_all", &morphio::ErrorAndWarningHandlerCollector::printAll, "ibid")
        .def("get_all", &morphio::ErrorAndWarningHandlerCollector::getAll, "ibid");

}
