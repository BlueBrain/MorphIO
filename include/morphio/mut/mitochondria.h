#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio
{
namespace mut
{
class MitoSection
{
public:
    MitoSection(const Property::MitochondriaPointLevel& mitoPoints)
        : _mitoPoints(mitoPoints)
    {
    }
    std::vector<float>& diameters() { return _mitoPoints._diameters; }
    std::vector<uint32_t>& neuriteSectionIds() { return _mitoPoints._sectionIds; }
    std::vector<float>& pathLengths() { return _mitoPoints._relativePathLengths; }

    Property::MitochondriaPointLevel _mitoPoints;
};

class Mitochondria
{
public:
    Mitochondria()
        : _mitochondriaCounter(-1)
        , _mitochondriaSectionCounter(-1)
    {
    }

    const std::vector<uint32_t> children(uint32_t id) const;
    const std::shared_ptr<MitoSection> section(uint32_t id) const;
    const std::map<uint32_t, std::shared_ptr<MitoSection>>& sections() const;
    const int32_t parent(uint32_t id) const;
    const std::vector<uint32_t>& rootSections() const;

    uint32_t appendSection(int32_t mitoParentId,
                           const Property::MitochondriaPointLevel& points);

    const std::shared_ptr<MitoSection> mitoSection(uint32_t id) const;

    void _buildMitochondria(Property::Properties& properties) const;

private:
    std::map<uint32_t, std::vector<uint32_t>> _children;
    std::map<uint32_t, uint32_t> _parent;
    std::vector<uint32_t> _rootSections;
    std::map<uint32_t, std::shared_ptr<MitoSection>> _sections;

    int32_t _mitochondriaCounter;
    int32_t _mitochondriaSectionCounter;
};
}
}
