#pragma once

#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio {
class GlialCell: public Morphology
{
  public:
    GlialCell(const std::string& source);

  private:
    Soma soma() const;
};
}  // namespace morphio
