#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

/** Mutable(editable) morphio::DendriticSpine */
class DendriticSpine: public Morphology
{
  public:
    DendriticSpine();
    explicit DendriticSpine(const std::string& source);

    std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() noexcept;
    const std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() const
        noexcept;
};

}  // namespace mut
}  // namespace morphio
