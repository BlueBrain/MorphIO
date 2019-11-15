#include <utility>

#include <pybind11/pybind11.h>

#include <morphio/types.h>
#include <morphio/enums.h>

namespace py = pybind11;

static py::array_t<float> span_array_to_ndarray(const morphio::range<const std::array<float, 3> > &span)
{
    const void* ptr = static_cast<const void*>(span.data());
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        const_cast<void*>(ptr),                            /* Pointer to buffer */
        sizeof(float),                          /* Size of one scalar */
        py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
        2,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        { static_cast<int>(span.size()), 3 }, /* buffer dimentions */
        { sizeof(float) * 3,                  /* Strides (in bytes) for each index */
                sizeof(float) }
        );
    return py::array(buffer_info);
}

template <typename T>
py::array_t<float> span_to_ndarray(const morphio::range<const T>& span)
{
    const void* ptr = static_cast<const void*>(span.data());
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        const_cast<void*>(ptr),                            /* Pointer to buffer */
        sizeof(T),                          /* Size of one scalar */
        py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
        1,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        {static_cast<int>(span.size())}, /* buffer dimentions */
        {sizeof(T)} );      /* Strides (in bytes) for each index */
    return py::array(buffer_info);
}


static void _raise_if_wrong_shape(const py::buffer_info& info) {
    const auto &shape = info.shape;
    if(shape.size() != 2 || info.shape[1] != 3) {
        std::string shape_str;
        for(unsigned int i=0; i<shape.size(); ++i){
            shape_str += std::to_string(shape[i]);
            if(i != shape.size() -1)
                shape_str += ", ";
        }
        throw morphio::MorphioError("Wrong array shape. Expected: (X, 3), got: (" + shape_str + ")");
    }
}

static morphio::Points array_to_points(py::array_t<float> &buf){
    morphio::Points points;
    py::buffer_info info = buf.request();
    _raise_if_wrong_shape(info);

    for(int i = 0;i<info.shape[0]; ++i){
        points.push_back(std::array<float, 3>{*buf.data(i, 0), *buf.data(i, 1), *buf.data(i, 2)});
    }
    return points;
}

/**
 * @brief "Casts" a Cpp sequence to a python array (no memory copies)
 *  Python capsule handles void pointers to objects and makes sure
 *      that they will remain alive.
 *
 *      https://github.com/pybind/pybind11/issues/1042#issuecomment-325941022
 */
template <typename Sequence>
inline py::array_t<typename Sequence::value_type> as_pyarray(Sequence&& seq) {
    // Move entire object to heap. Memory handled via Python capsule
    Sequence* seq_ptr = new Sequence(std::move(seq));
    // Capsule shall delete sequence object when done
    auto capsule = py::capsule(seq_ptr,
                               [](void* p) { delete reinterpret_cast<Sequence*>(p); });

    return py::array(static_cast<py::ssize_t>(seq_ptr->size()),  // shape of array
                     seq_ptr->data(),  // c-style contiguous strides for Sequence
                     capsule           // numpy array references this parent
    );
}
