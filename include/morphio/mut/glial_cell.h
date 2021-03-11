#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {

class GlialCell;

namespace mut {

class GlialCell: public Morphology
{
  public:
    GlialCell();
    GlialCell(const std::string& source);
     /**
       Build a mutable GlialCell from a read-only glialCell
    **/
    GlialCell(const morphio::GlialCell& gliaCell, unsigned int options = NO_MODIFIER);
};

}  // namespace mut
}  // namespace morphio
