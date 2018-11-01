#pragma once

#include <functional>

#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

#include <morphio/mut/iterators.h>

namespace morphio
{
namespace mut
{

class Section: public std::enable_shared_from_this<Section>
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
    const bool isRoot() const;

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

    std::shared_ptr<Section> appendSection(const Property::PointLevel&,
                                           SectionType sectionType = SectionType::SECTION_UNDEFINED);


private:
    friend class Morphology;

    // The joy of C++:
    // https://stackoverflow.com/questions/8202530/how-can-i-call-a-private-destructor-from-a-shared-ptr
    friend void friendDtorForSharedPtr(Section *);

    Section(Morphology*, int id, SectionType type, const Property::PointLevel&);
    Section(Morphology*, int id, const morphio::Section& section);
    Section(Morphology*, int id, const Section&);

    Property::PointLevel _pointProperties;
    SectionType _sectionType;
    Morphology* _morphology;
    uint32_t _id;
};

void friendDtorForSharedPtr(Section* section);

std::ostream& operator<<(std::ostream& os, Section&);
std::ostream& operator<<(std::ostream& os, std::shared_ptr<Section>);

} // namespace mut
} // namespace morphio
