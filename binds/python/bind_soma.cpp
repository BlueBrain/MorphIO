
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <morphio/soma.h>
#include <morphio/types.h>

#include "bind_soma.h"
#include "bindings_utils.h"

namespace py = pybind11;

void bind_soma_module(py::module& m) {
    py::class_<morphio::Soma, std::shared_ptr<morphio::Soma>>(m, "Soma")
        .def(py::init<const morphio::Soma&>())

        .def(py::init<const morphio::Property::PointLevel&>())

        .def_property(
            "points",
            [](morphio::Soma* soma) {
                return py::array(static_cast<py::ssize_t>(soma->points().size()),
                                 soma->points().data());
            },
            [](morphio::Soma* soma, py::array_t<morphio::floatType> points) {
                soma->points() = array_to_points(points);
            },
            "Returns the coordinates (x,y,z) of all soma point")

        .def_property(
            "diameters",
            [](morphio::Soma* soma) {
                return py::array(static_cast<py::ssize_t>(soma->diameters().size()),
                                 soma->diameters().data());
            },
            [](morphio::Soma* soma, py::array_t<morphio::floatType> _diameters) {
                soma->diameters() = _diameters.cast<std::vector<morphio::floatType>>();
            },
            "Returns the diameters of all soma points")

        .def_property_readonly(
            "center",
            [](morphio::Soma* soma) { return py::array(3, soma->center().data()); },
            "Returns the center of gravity of the soma points")

        .def_property_readonly("max_distance",
                               &morphio::Soma::maxDistance,
                               "Return the maximum distance between the center of gravity "
                               "and any of the soma points")

        .def_property_readonly("type", &morphio::Soma::type, "Returns the soma type")

        .def_property_readonly("surface",
                               &morphio::Soma::surface,
                               "Returns the soma surface\n\n"
                               "Note: the soma surface computation depends on the soma type");
}
