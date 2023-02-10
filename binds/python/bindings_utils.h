#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <morphio/types.h>

#include <morphio/dendritic_spine.h>
#include <morphio/endoplasmic_reticulum.h>
#include <morphio/glial_cell.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/mut/dendritic_spine.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/soma.h>
#include <morphio/soma.h>


namespace py = pybind11;

morphio::Points array_to_points(py::array_t<morphio::floatType>& buf);
py::array_t<morphio::floatType> span_array_to_ndarray(
    const morphio::range<const morphio::Point>& span);

template <typename T>
py::array_t<morphio::floatType> span_to_ndarray(const morphio::range<const T>& span) {
    const void* ptr = static_cast<const void*>(span.data());
    const auto buffer_info = py::buffer_info(
        // Cast from (const void*) to (void*) for function signature matching
        const_cast<void*>(ptr),             /* Pointer to buffer */
        sizeof(T),                          /* Size of one scalar */
        py::format_descriptor<T>::format(), /* Python struct-style format descriptor */
        1,                                  /* Number of dimensions */

        // Forced cast to prevent error:
        // template argument deduction/substitution failed */
        {static_cast<int>(span.size())}, /* buffer dimentions */
        {sizeof(T)});                    /* Strides (in bytes) for each index */
    return py::array(buffer_info);
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
    auto capsule = py::capsule(seq_ptr, [](void* p) { delete reinterpret_cast<Sequence*>(p); });

    return py::array(static_cast<py::ssize_t>(seq_ptr->size()),  // shape of array
                     seq_ptr->data(),  // c-style contiguous strides for Sequence
                     capsule           // numpy array references this parent
    );
}

struct immutable_binding_classes {
    // Immutable classes
    py::class_<morphio::Morphology> Morphology_class;
    py::class_<morphio::GlialCell, morphio::Morphology> GlialCell_class;
    py::class_<morphio::Mitochondria> Mitochondria_class;
    py::class_<morphio::EndoplasmicReticulum> EndoplasmicReticulum_class;
    py::class_<morphio::Soma> Soma_class;
    py::class_<morphio::Section> Section_class;
    py::class_<morphio::MitoSection> MitoSection_class;
    py::class_<morphio::DendriticSpine, morphio::Morphology> DendriticSpine_class;
};

struct mutable_binding_classes {
    // Mutable classes
    py::class_<morphio::mut::Morphology> Morphology_mut_class;
    py::class_<morphio::mut::GlialCell, morphio::mut::Morphology> GlialCell_mut_class;
    py::class_<morphio::mut::Mitochondria> Mitochondria_mut_class;
    py::class_<morphio::mut::MitoSection, std::shared_ptr<morphio::mut::MitoSection>>
        MitoSection_mut_class;
    py::class_<morphio::mut::Section, std::shared_ptr<morphio::mut::Section>> Section_mut_class;
    py::class_<morphio::mut::Soma, std::shared_ptr<morphio::mut::Soma>> Soma_mut_class;
    py::class_<morphio::mut::EndoplasmicReticulum> EndoplasmicReticulum_mut_class;
    py::class_<morphio::mut::DendriticSpine, morphio::mut::Morphology> DendriticSpine_mut_class;
};
