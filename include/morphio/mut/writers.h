#include <morphio/mut/morphology.h>

namespace morphio {
namespace mut {
namespace writer {
/** Save morphology in SWC format */
void swc(const Morphology& morphology, const std::string& filename);
/** Save morphology in ASC format */
void asc(const Morphology& morphology, const std::string& filename);
/** Save morphology in H5 format */
void h5(const Morphology& morphology, const std::string& filename);
}  // namespace writer
}  // end namespace mut
}  // end namespace morphio
