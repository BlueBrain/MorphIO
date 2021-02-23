#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

class GlialCell: public Morphology
{
  public:
    GlialCell();
    void init(const std::string& uri, unsigned int options = NO_MODIFIER);
};

}  // namespace mut
}  // namespace morphio
