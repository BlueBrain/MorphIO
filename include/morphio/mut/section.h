#pragma once

#include <functional>

#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

namespace morphio
{
namespace mut
{

class Section
{
public:
    // uint32_t const parent();
    // const std::set<uint32_t>& children();
    const uint32_t id() const { return _id; }
    SectionType& type() { return _sectionType; }
    std::vector<Point>& points() { return _pointProperties._points; }
    std::vector<float>& diameters() { return _pointProperties._diameters; }
    std::vector<float>& perimeters() { return _pointProperties._perimeters; }

private:
    friend class Morphology;

    // The joy of C++:
    // https://stackoverflow.com/questions/8202530/how-can-i-call-a-private-destructor-from-a-shared-ptr
    friend void friendDtorForSharedPtr(Section *);

    void traverse(Morphology& morphology,
                  std::function<void(Morphology& morphology, uint32_t sectionId)>);
    Section(int id, SectionType type, const Property::PointLevel&);
    Section(const morphio::Section& section);
    ~Section() {}

    Property::PointLevel _pointProperties;
    SectionType _sectionType;
    uint32_t _id;
};

void friendDtorForSharedPtr(Section* section);

std::ostream& operator<<(std::ostream& os, Section&);
std::ostream& operator<<(std::ostream& os, std::shared_ptr<Section>);

} // namespace mut
} // namespace morphio
