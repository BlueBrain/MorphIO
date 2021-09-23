#pragma once

#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio {

/** Class to represent morphologies of glial cells */
class GlialCell: public Morphology
{
  public:
    GlialCell(const std::string& source);

  private:
    Soma soma() const;
};
}  // namespace morphio
