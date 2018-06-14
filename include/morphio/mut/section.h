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

    /**
     * Return the section ID
     **/
    const uint32_t id() const { return _id; }

    /**
     * Return the morphological type of this section (dendrite, axon, ...)
     **/
    SectionType& type() { return _sectionType; }

    /**
       Return the coordinates (x,y,z) of all points of this section
    **/
    std::vector<Point>& points() { return _pointProperties._points; }

    /**
       Return the diameters of all points of this section
    **/
    std::vector<float>& diameters() { return _pointProperties._diameters; }

    /**
       Return the perimeters of all points of this section
    **/
    std::vector<float>& perimeters() { return _pointProperties._perimeters; }

    /**
       Return the PointLevel instance that contains this section's data
    **/
    Property::PointLevel& properties() { return _pointProperties; }

    ~Section() {}

private:
    friend class Morphology;

    // The joy of C++:
    // https://stackoverflow.com/questions/8202530/how-can-i-call-a-private-destructor-from-a-shared-ptr
    friend void friendDtorForSharedPtr(Section *);

    Section(int id, SectionType type, const Property::PointLevel&);
    Section(int id, const morphio::Section& section);
    Section(int id, const Section&);

    Property::PointLevel _pointProperties;
    SectionType _sectionType;
    uint32_t _id;
};

void friendDtorForSharedPtr(Section* section);

std::ostream& operator<<(std::ostream& os, Section&);
std::ostream& operator<<(std::ostream& os, std::shared_ptr<Section>);

} // namespace mut
} // namespace morphio
