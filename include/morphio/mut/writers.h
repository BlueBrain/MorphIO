#include <morphio/mut/morphology.h>

namespace morphio {
namespace mut {
namespace writer {
void swc(const Morphology& morphology, const std::string& filename);
void asc(const Morphology& morphology, const std::string& filename);

template <typename Cell>
void h5(const Cell& cell, const std::string& filename);

extern template void h5(const Morphology& cell, const std::string& filename);
extern template void h5(const GlialCell& cell, const std::string& filename);


}  // namespace writer
}  // end namespace mut
}  // end namespace morphio
