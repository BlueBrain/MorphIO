#include <morphio/mut/morphology.h>

namespace morphio {
namespace mut {
namespace writer {
void swc(const Morphology& morphology, const std::string& filename);
void asc(const Morphology& morphology, const std::string& filename);
void h5(const Morphology& morphology, const std::string& filename);
}  // namespace writer
}  // end namespace mut
}  // end namespace morphio
