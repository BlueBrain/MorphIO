#pragma once

#include <morphio/types.h>

namespace morphio {
/**
 * The entry-point class to access mitochondrial data
 *
 * By design, it is the equivalent of the Morphology class but at the
 *mitochondrial level. As the Morphology class, it implements a section accessor
 *and a root section accessor returning views on the Properties object for the
 *queried mitochondrial section.
 **/
class Mitochondria
{
  public:
    MitoSection section(uint32_t id) const;
    std::vector<MitoSection> rootSections() const;
    std::vector<MitoSection> sections() const;

  private:
    explicit Mitochondria(const std::shared_ptr<Property::Properties>& properties)
        : _properties(properties) {}
    std::shared_ptr<Property::Properties> _properties;

    friend class Morphology;
};
}  // namespace morphio
