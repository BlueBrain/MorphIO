#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

/** Mutable(editable) morphio::GlialCell */
class GlialCell: public Morphology
{
  public:
    GlialCell();
    GlialCell(const std::string& source);
};

}  // namespace mut
}  // namespace morphio
