#include <morphio/mut/morphology.h>

namespace morphio {
namespace mut {
namespace writer {
template <typename Type>
void swc(const TMorphology<Type>& morphology, const std::string& filename);
template <typename Type>
void asc(const TMorphology<Type>& morphology, const std::string& filename);
template <typename Type>
void h5(const TMorphology<Type>& morphology, const std::string& filename);
}  // namespace writer
}  // end namespace mut
}  // end namespace morphio
