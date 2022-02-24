#pragma once

#include <string>  // std::string

#include <morphio/mut/morphology.h>  // mut::Morphology

namespace morphio {
namespace mut {

/** Mutable(editable) morphio::GlialCell */
class GlialCell: public Morphology
{
  public:
    GlialCell();
    explicit GlialCell(const std::string& source);
};

}  // namespace mut
}  // namespace morphio
