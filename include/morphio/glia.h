#pragma once

#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio {
class Glia: public Morphology
{
  public:
    Glia(const std::string& source);

  private:
    Soma soma() const;
};
}  // namespace morphio
