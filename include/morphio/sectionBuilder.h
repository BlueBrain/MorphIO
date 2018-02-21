#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>

#include <functional>

#include <gsl/span>

#include <morphio/exceptions.h>
#include <morphio/iterators.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>

namespace morphio
{
namespace builder
{
class Soma
{
public:
    Soma()
    {
    }
    Soma(Property::PointLevel pointProperties):
        _pointProperties(pointProperties)
    {
    }

    Soma(const morphio::Soma& soma);
    std::vector<Point>& points() { return _pointProperties._points; }
    std::vector<float> diameters() { return _pointProperties._diameters; }
private:
    friend class Morphology;
    Property::PointLevel _pointProperties;
};

class Section
{
public:
    Section* const parent();
    const std::set<Section*>& children();
    const uint32_t id() const { return _id; }
    SectionType& type() { return _sectionType; }
    std::vector<Point>& points() { return _pointProperties._points; }
    std::vector<float> diameters() { return _pointProperties._diameters; }
    std::vector<float> perimeters() { return _pointProperties._perimeters; }
private:
    void traverse(
        Morphology* morphology,
        std::function<void(Morphology* morphology, Section* section)>);
    Section(Morphology*, int id, SectionType type, const Property::PointLevel&);
    Section(Morphology* morphology, const morphio::Section& section,
            bool recursive = true);
    ~Section() {}
    friend class Morphology;
    Property::PointLevel _pointProperties;
    SectionType _sectionType;
    Section* _parent;
    std::set<Section*> _children;
    uint32_t _id;
};

class Morphology
{
public:
    Morphology()
        : _soma(Soma())
        , _counter(0)
    {
    }
    Morphology(const morphio::Morphology& morphology);
    virtual ~Morphology();
    const std::set<Section*>& rootSections();

    std::map<uint32_t, Section*>& sections() { return _sections; }
    Soma& soma();
    void deleteSection(Section*, bool recursive = true);
    uint32_t appendSection(Section* parent, const morphio::Section&,
                           bool recursive = true);
    uint32_t appendSection(Section* parent, SectionType,
                           const Property::PointLevel&);
    uint32_t createNeurite(const morphio::Section&, bool recursive = true);
    uint32_t createNeurite(SectionType, const Property::PointLevel&);
    void traverse(std::function<void(Morphology* morphology, Section* section)>,
                  Section* rootSection = nullptr);

    Property::Properties buildReadOnly();

private:
    friend class Section;

    void _register(Section*);
    Soma _soma;
    std::set<Section*> _rootSections;
    std::map<uint32_t, Section*> _sections;
    uint32_t _counter;
};

namespace writer
{
void swc(Morphology& morphology);
void asc(Morphology& morphology);
void h5(Morphology& morphology);
}
}
}
