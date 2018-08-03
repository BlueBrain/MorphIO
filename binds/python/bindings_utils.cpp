#include <pybind11/pybind11.h>
#include <morphio/types.h>
#include <morphio/enums.h>

namespace py = pybind11;

py::array_t<float> span_array_to_ndarray(const morphio::range<const std::array<float, 3> > &span)
{
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        (void*)span.data(),                            /* Pointer to buffer */
        sizeof(float),                          /* Size of one scalar */
        py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
        2,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        { (int) span.size(), 3 }, /* buffer dimentions */
        { sizeof(float) * 3,                  /* Strides (in bytes) for each index */
                sizeof(float) }
        );
    return py::array(buffer_info);
}

template <typename T>
py::array_t<float> span_to_ndarray(const morphio::range<const T>& span)
{
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        (void*)span.data(),                            /* Pointer to buffer */
        sizeof(T),                          /* Size of one scalar */
        py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
        1,                                      /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        {(int) span.size()}, /* buffer dimentions */
        {sizeof(T)} );      /* Strides (in bytes) for each index */
    return py::array(buffer_info);
}


morphio::Points array_to_points(py::array_t<float> &buf){
    morphio::Points points;
    py::buffer_info info = buf.request();
    float* ptr = (float*)info.ptr;

    for(int i = 0;i<info.shape[0]; ++i){
        points.push_back(std::array<float, 3>{ptr[3 * i],
                    ptr[3 * i + 1],
                    ptr[3 * i + 2]});
    }
    return points;
}
