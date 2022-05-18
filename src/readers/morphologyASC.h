#pragma once
#include <morphio/types.h>

namespace morphio {
namespace readers {
namespace asc {
Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options);
}  // namespace asc
}  // namespace readers
}  // namespace morphio
