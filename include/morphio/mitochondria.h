#pragma once

#include <morphio/types.h>

namespace morphio
{


class Mitochondria
{
public:
    const MitoSection section(const uint32_t& id) const;
    const std::vector<MitoSection> rootSections() const;
    const std::vector<MitoSection> sections() const;

private:
    Mitochondria(std::shared_ptr<Property::Properties> properties) : _properties(properties) {}
    std::shared_ptr<Property::Properties> _properties;

    friend class Morphology;
};
}
