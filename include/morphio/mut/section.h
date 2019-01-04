#pragma once

#include <functional>

#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

#include <morphio/mut/iterators.h>

namespace morphio {
namespace mut {
class Section : public std::enable_shared_from_this<Section>
{
public:
    /**
     * Return the section ID
     **/
    uint32_t id() const { return _id; }

    /**
     * Return the morphological type of this section (dendrite, axon, ...)
     **/
    SectionType& type() { return _sectionType; }
    const SectionType& type() const { return _sectionType; }

    /**
       Return the coordinates (x,y,z) of all points of this section
    **/
    std::vector<Point>& points() { return _pointProperties._points; }
    const std::vector<Point>& points() const
    {
        return _pointProperties._points;
    }

    /**
       Return the diameters of all points of this section
    **/
    std::vector<float>& diameters() { return _pointProperties._diameters; }
    const std::vector<float>& diameters() const
    {
        return _pointProperties._diameters;
    }

    /**
       Return the perimeters of all points of this section
    **/
    std::vector<float>& perimeters() { return _pointProperties._perimeters; }
    const std::vector<float>& perimeters() const
    {
        return _pointProperties._perimeters;
    }

    /**
       Return the PointLevel instance that contains this section's data
    **/
    Property::PointLevel& properties() { return _pointProperties; }

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Methods that were previously in mut::Morphology
    //
    ////////////////////////////////////////////////////////////////////////////////

    /**
       Get the parent ID

       Note: Root sections return -1
    **/
    const std::shared_ptr<Section> parent() const;

    /**
       Return true if section is a root section
    **/
    bool isRoot() const;

    /**
       Return a vector of children IDs
    **/
    const std::vector<std::shared_ptr<Section>> children() const;

    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;

    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

    upstream_iterator upstream_begin() const;
    upstream_iterator upstream_end() const;

    ~Section() {}

    std::shared_ptr<Section> appendSection(const morphio::Section&,
        bool recursive = false);

    std::shared_ptr<Section> appendSection(std::shared_ptr<Section> section,
        bool recursive = false);

    std::shared_ptr<Section> appendSection(
        const Property::PointLevel&,
        SectionType sectionType = SectionType::SECTION_UNDEFINED);

    /**
    Two sections are equal if they have same:
    - points
    - diameters
    - perimeters
    - type
    **/
    bool operator==(const Section& other) const;
    bool operator!=(const Section& other) const;

private:
    friend class Morphology;

    // The joy of C++:
    // https://stackoverflow.com/questions/8202530/how-can-i-call-a-private-destructor-from-a-shared-ptr
    friend void friendDtorForSharedPtr(Section*);

    Section(Morphology*, int id, SectionType type, const Property::PointLevel&);
    Section(Morphology*, int id, const morphio::Section& section);
    Section(Morphology*, int id, const Section&);
    bool _compare(const Section&, bool) const;

    Morphology* _morphology;
    Property::PointLevel _pointProperties;
    uint32_t _id;
    SectionType _sectionType;
};

void friendDtorForSharedPtr(Section* section);

std::ostream& operator<<(std::ostream& os, Section&);
std::ostream& operator<<(std::ostream& os, std::shared_ptr<Section>);

} // namespace mut
} // namespace morphio
