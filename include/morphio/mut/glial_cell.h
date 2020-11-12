#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

class GlialCell: public Morphology
{
  public:
    GlialCell();
    GlialCell(const std::string& source);
};

}  // namespace mut
}  // namespace morphio
