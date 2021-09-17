#pragma once

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {

class DendriticSpine: public Morphology
{
  public:
    DendriticSpine();
    DendriticSpine(const std::string& source);

    std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() noexcept;
    const std::vector<Property::DendriticSpine::PostSynapticDensity>& postSynapticDensity() const
        noexcept;
};

}  // namespace mut
}  // namespace morphio
