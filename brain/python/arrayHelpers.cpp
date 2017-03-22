/* Copyright (c) 2013-2016, Juan Hernando <juan.hernando@epfl.ch>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "arrayHelpers.h"
#include "../neuron/types.h"
#include "../types.h"
#include "numpy.h"

#include "brain/compartmentReport.h"
#include "brain/compartmentReportMapping.h"

#include <vmmlib/matrix.hpp>
#include <vmmlib/quaternion.hpp>
#include <vmmlib/vector.hpp>

namespace bp = boost::python;

namespace brain
{
namespace
{
// Helper clasess for template meta-programming of create_array_from_vector
template <typename T>
struct NumpyArrayInfo;

#define DECLARE_ARRAY_INFO(T, NPY_TYPE, NUM_DIM, /*DIMENSIONS*/...) \
    template <>                                                     \
    struct NumpyArrayInfo<T>                                        \
    {                                                               \
        NumpyArrayInfo(const npy_intp size)                         \
            : descr(PyArray_DescrFromType(NPY_TYPE))                \
            , dims{size, __VA_ARGS__}                               \
        {                                                           \
        }                                                           \
        static const int ndims = NUM_DIM;                           \
        PyArray_Descr* descr;                                       \
        npy_intp dims[ndims];                                       \
    };

PyArray_Descr* createDtype(const char* dtype)
{
    bp::object format(dtype);
    PyArray_Descr* descr;
    if (PyArray_DescrConverter(format.ptr(), &descr) == -1)
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "Internal wrapping error in C++ to"
                        " numpy array conversion");
        bp::throw_error_already_set();
    }
    return descr;
}

#define DECLARE_STRUCTURED_ARRAY_INFO(T, DTYPE_STR) \
    template <>                                     \
    struct NumpyArrayInfo<T>                        \
    {                                               \
        NumpyArrayInfo(const npy_intp size)         \
            : descr(createDtype(DTYPE_STR))         \
            , dims{size}                            \
        {                                           \
        }                                           \
        static const int ndims = 1;                 \
        PyArray_Descr* descr;                       \
        npy_intp dims[1];                           \
    };

DECLARE_ARRAY_INFO(uint16_t, NPY_USHORT, 1)
DECLARE_ARRAY_INFO(uint32_t, NPY_UINT, 1)
DECLARE_ARRAY_INFO(int, NPY_INT, 1)
DECLARE_ARRAY_INFO(size_t, NPY_LONG, 1)
DECLARE_ARRAY_INFO(float, NPY_FLOAT, 1)
DECLARE_ARRAY_INFO(double, NPY_DOUBLE, 1)
DECLARE_ARRAY_INFO(neuron::SectionType, NPY_INT, 1)
DECLARE_ARRAY_INFO(Vector2i, NPY_INT, 2, 2)
DECLARE_ARRAY_INFO(Vector3f, NPY_FLOAT, 2, 3)
DECLARE_ARRAY_INFO(Vector4f, NPY_FLOAT, 2, 4)
DECLARE_ARRAY_INFO(Quaternionf, NPY_FLOAT, 2, 4)
DECLARE_ARRAY_INFO(Matrix4f, NPY_FLOAT, 3, 4, 4)
DECLARE_STRUCTURED_ARRAY_INFO(CompartmentReportMapping::IndexEntry,
                              "u8, u4, u2, u2")
DECLARE_STRUCTURED_ARRAY_INFO(Spike, "f4, u4")

// Functions for the boost::shared_ptr< std::vector< T >> to numpy converter

void _setBaseObject(PyObject* array, const AbstractCustodianPtr& keeper)
{
    // Pointer sharing between the C++ shrared_ptr and the numpy array is
    // only available for Numpy >= 1.7.
    // PyArray_SetBaseObject steels the reference.
    bp::object pykeeper(keeper);
    Py_INCREF(pykeeper.ptr());
    if (PyArray_SetBaseObject((PyArrayObject*)array, pykeeper.ptr()) == -1)
    {
        // The reference is not clear about the error string having been
        // set or not. Let's assume it has been.
        Py_DECREF(array);
        Py_DECREF(pykeeper.ptr());
        bp::throw_error_already_set();
    }
}

template <typename T>
PyObject* _createNumpyArray(const T* data, const size_t size,
                            const AbstractCustodianPtr& keeper)
{
    typedef NumpyArrayInfo<T> Info;
    Info info(size);
    PyObject* array =
        PyArray_NewFromDescr(&PyArray_Type, info.descr, Info::ndims, info.dims,
                             nullptr, (void*)data, 0, nullptr);
    _setBaseObject(array, keeper);
    return array;
}

template <typename T>
PyObject* _createNumpyArray(const std::vector<T>& vector,
                            const AbstractCustodianPtr& keeper)
{
    return _createNumpyArray(vector.data(), vector.size(), keeper);
}

template <typename T>
struct VectorTsToNumpyArray
{
    static PyObject* convert(const VectorPtr<T>& input)
    {
        AbstractCustodianPtr custodian(new Custodian<VectorPtr<T>>(input));
        return _createNumpyArray(*input, custodian);
    }
};

template <typename T>
struct ConstVectorTsWithCustodianToNumpyArray
{
    static PyObject* convert(const ConstVectorWithCustodian<T>& input)
    {
        PyObject* object = _createNumpyArray(input.vector, input.custodian);
        PyArrayObject* array = (PyArrayObject*)object;
        PyArray_CLEARFLAGS(array, NPY_ARRAY_WRITEABLE);
        return object;
    }
};

template <typename T>
struct ConstArrayTsWithCustodianToNumpyArray
{
    static PyObject* convert(const ConstArrayWithCustodian<T>& input)
    {
        PyObject* object =
            _createNumpyArray(input.array, input.size, input.custodian);
        PyArrayObject* array = (PyArrayObject*)object;
        PyArray_CLEARFLAGS(array, NPY_ARRAY_WRITEABLE);
        return object;
    }
};
}

// Import and registrations

#if PY_VERSION_HEX >= 0x03000000
void* _importArray()
{
    import_array();
    return PyArray_API;
}
#else
void _importArray()
{
    import_array();
}
#endif

#define REGISTER_ARRAY_CONVERTER(type)                                       \
    bp::to_python_converter<VectorPtr<type>, VectorTsToNumpyArray<type>>();  \
    bp::to_python_converter<ConstVectorWithCustodian<type>,                  \
                            ConstVectorTsWithCustodianToNumpyArray<type>>(); \
    bp::to_python_converter<ConstArrayWithCustodian<type>,                   \
                            ConstArrayTsWithCustodianToNumpyArray<type>>();

void importArray()
{
    _importArray();

    REGISTER_ARRAY_CONVERTER(uint16_t);
    REGISTER_ARRAY_CONVERTER(uint32_t);
    REGISTER_ARRAY_CONVERTER(int);
    REGISTER_ARRAY_CONVERTER(size_t);
    REGISTER_ARRAY_CONVERTER(float);
    REGISTER_ARRAY_CONVERTER(double);
    REGISTER_ARRAY_CONVERTER(neuron::SectionType);
    REGISTER_ARRAY_CONVERTER(CompartmentReportMapping::IndexEntry);
    REGISTER_ARRAY_CONVERTER(Matrix4f);
    REGISTER_ARRAY_CONVERTER(Quaternionf);
    REGISTER_ARRAY_CONVERTER(Spike);
    REGISTER_ARRAY_CONVERTER(Vector2i);
    REGISTER_ARRAY_CONVERTER(Vector3f);
    REGISTER_ARRAY_CONVERTER(Vector4f);

    bp::class_<AbstractCustodian, AbstractCustodianPtr>("_Custodian");
}

bool isArray(const bp::object& o)
{
    return PyArray_Check(o.ptr());
}

bp::object toNumpy(const brain::Matrix4f& matrix)
{
    npy_intp dims[2] = {4, 4};
    void* data = malloc(sizeof(float) * 16);
    if (!data)
    {
        PyErr_SetString(PyExc_MemoryError,
                        "Allocating numpy array for Matrix4f");
        bp::throw_error_already_set();
    }
    memcpy(data, &matrix, sizeof(brain::Matrix4f));
    PyObject* array =
        PyArray_New(&PyArray_Type, 2, dims, NPY_FLOAT, 0, data, 0,
                    NPY_ARRAY_OWNDATA | NPY_ARRAY_F_CONTIGUOUS, 0);
    return bp::object(bp::handle<>(array));
}

bp::object frameToTuple(CompartmentReportFrame&& frame)
{
    if (frame.empty())
        return bp::object(); // None

    return bp::make_tuple(frame.getTimestamp(), toNumpy(frame.takeData()));
}

bp::object framesToTuple(brion::Frames&& frames)
{
    if (!frames.data)
        return bp::object(); // None

    size_t frameCount = frames.timeStamps->size();
    size_t frameSize = frames.data->size() / frameCount;

    // TODO :remove this when time stamps will be doubles in brion::CR
    std::vector<double> timestamps(frameCount);
    size_t index = 0;
    for (auto val : *frames.timeStamps)
        timestamps[index++] = val;

    auto data = frames.data;

    npy_intp dimensions[2] = {(npy_intp)frameCount, (npy_intp)frameSize};

    PyObject* array = PyArray_New(&PyArray_Type, 2, dimensions, NPY_FLOAT, 0,
                                  data->data(), 0, NPY_ARRAY_C_CONTIGUOUS, 0);

    bp::object pykeeper(data);
    Py_INCREF(pykeeper.ptr());
    if (PyArray_SetBaseObject((PyArrayObject*)array, pykeeper.ptr()) == -1)
    {
        // The reference is not clear about the error string having been
        // set or not. Let's assume it has been.
        Py_DECREF(array);
        Py_DECREF(pykeeper.ptr());
        bp::throw_error_already_set();
    }

    return bp::make_tuple(brain::toNumpy(std::move(timestamps)),
                          bp::handle<>(array));
}

namespace
{
template <typename T>
bool _copyGIDs(PyArrayObject* array, uint32_ts& result)
{
    const size_t size = PyArray_DIMS(array)[0];
    bool sorted = true;

    result.clear();
    result.reserve(size);

    T last = 0;
    for (size_t i = 0; i != size; ++i)
    {
        const T gid = *static_cast<T*>(PyArray_GETPTR1(array, i));
        if (gid < 0 ||
            ssize_t(gid) > ssize_t(std::numeric_limits<uint32_t>::max()))
        {
            PyErr_SetString(PyExc_ValueError, "Invalid input GID");
            boost::python::throw_error_already_set();
        }
        if (last >= gid)
            sorted = false;
        else
            last = gid;
        result.push_back(gid);
    }
    return sorted;
}

template <typename T>
void _copyArrayToMatrix(PyArrayObject* array, Matrix4f& matrix)
{
    for (size_t i = 0; i != 4; ++i)
        for (size_t j = 0; j != 4; ++j)
            matrix(i, j) = *static_cast<T*>(PyArray_GETPTR2(array, i, j));
}
}

bool gidsFromNumpy(const boost::python::object& object, uint32_ts& result)
{
    PyArrayObject* array = reinterpret_cast<PyArrayObject*>(object.ptr());
    if (PyArray_NDIM(array) != 1)
    {
        PyErr_SetString(PyExc_ValueError, "Cannot convert argument to GID set");
        boost::python::throw_error_already_set();
    }

    switch (PyArray_TYPE(array))
    {
    case NPY_LONG:
        return _copyGIDs<long>(array, result);
    case NPY_INT:
        return _copyGIDs<int>(array, result);
    case NPY_UINT:
        return _copyGIDs<unsigned int>(array, result);
    default:;
    }
    std::stringstream msg;
    PyArray_Descr* desc = PyArray_DESCR(array);
    msg << "Cannot convert numpy array of type " << desc->kind << desc->elsize
        << " to GID set" << std::endl;
    PyErr_SetString(PyExc_ValueError, msg.str().c_str());
    boost::python::throw_error_already_set();
    return false; // Unreachable
}

std::pair<const Spike*, size_t> spikesFromNumpy(
    const boost::python::object& object)
{
    if (!PyArray_Check(object.ptr()))
    {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot convert argument to Spike array");
        boost::python::throw_error_already_set();
    }

    PyArrayObject* array = reinterpret_cast<PyArrayObject*>(object.ptr());
    PyArray_Descr* desc = PyArray_DESCR(array);
    PyArray_Descr* expected = createDtype("f4, u4");
    const bool equiv = PyArray_EquivTypes(desc, expected);
    Py_DECREF(expected);
    if (PyArray_NDIM(array) != 1 || !equiv)
    {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot convert argument to Spike array");
        boost::python::throw_error_already_set();
    }

    return std::pair<const Spike*, size_t>(static_cast<Spike*>(
                                               PyArray_GETPTR1(array, 0)),
                                           PyArray_DIMS(array)[0]);
}

template <>
Matrix4f fromNumpy(const bp::object& o)
{
    if (!isArray(o))
    {
        PyErr_SetString(PyExc_ValueError, "Cannot convert object to Matrix4f");
        bp::throw_error_already_set();
    }
    PyArrayObject* array = reinterpret_cast<PyArrayObject*>(o.ptr());

    if (PyArray_NDIM(array) != 2 || PyArray_DIMS(array)[0] != 4 ||
        PyArray_DIMS(array)[0] != 4)
    {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot convert array to Matrix4f, bad dimensions");
        bp::throw_error_already_set();
    }

    Matrix4f result;
    switch (PyArray_TYPE(array))
    {
    case NPY_FLOAT:
        _copyArrayToMatrix<float>(array, result);
        break;
    case NPY_DOUBLE:
        _copyArrayToMatrix<double>(array, result);
        break;
    case NPY_LONG:
        _copyArrayToMatrix<long>(array, result);
        break;
    case NPY_INT:
        _copyArrayToMatrix<int>(array, result);
        break;
    case NPY_UINT:
        _copyArrayToMatrix<unsigned int>(array, result);
        break;
    default:
    {
        std::stringstream msg;
        PyArray_Descr* desc = PyArray_DESCR(array);
        msg << "Cannot convert numpy array of type " << desc->kind
            << desc->elsize << " into Matrix4f" << std::endl;
        PyErr_SetString(PyExc_ValueError, msg.str().c_str());
        bp::throw_error_already_set();
    }
    }
    return result;
}
}
