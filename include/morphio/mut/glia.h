#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

class Glia: public Morphology
{
  public:
    Glia();
    Glia(const std::string& source);
};

}  // namespace mut
}  // namespace morphio
