#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace plugin {
namespace swc {
Property::Properties load(const URI& uri, unsigned int options);
} // namespace swc

} // namespace plugin

} // namespace morphio
