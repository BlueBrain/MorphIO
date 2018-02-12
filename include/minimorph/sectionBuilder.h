#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <memory>

#include <functional>

#include <gsl/span>

#include <minimorph/api.h>
#include <minimorph/types.h>
#include <minimorph/section.h>
#include <minimorph/properties.h>
#include <minimorph/exceptions.h>
#include <minimorph/iterators.h>


namespace minimorph
{

namespace builder
{

class Soma
{
public:
    Soma(const minimorph::Soma &soma);

private:
    Property::PointLevel _pointProperties;
    SectionType _somaType;
};

class Section
{
public:
    const Section* const getParent();
    const std::set<Section*>& getChildren();
    const uint32_t getID() const { return _id; }
    const SectionType getType() const { return _sectionType; }

    std::vector<Point>& getPoints(){ return _pointProperties._points;}
    std::vector<float> getDiameters(){ return _pointProperties._diameters;}
    std::vector<float> getPerimeters(){ return _pointProperties._perimeters;}

private:
    void traverse(Morphology* morphology, void (*fun)(Morphology* morphology, Section* section));
    Section(Morphology*, int id, SectionType type, const Property::PointLevel&);
    Section(Morphology* morphology, const minimorph::Section &section, bool recursive = true);
    ~Section(){}

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
    Morphology(const minimorph::Morphology& morphology);
    const std::set<Section*>& getRootSections() {
        return _rootSections;
    }

    void deleteSection(Section*, bool recursive = true);
    uint32_t appendSection(Section* parent, const minimorph::Section&, bool recursive=true);
    uint32_t appendSection(Section* parent, SectionType, const Property::PointLevel&);
    void traverse(void (*fun)(Morphology* morphology, Section* section), Section* rootSection = nullptr);
private:
    friend class Section;

    void _register(Section*);
    Soma _soma;
    std::set<Section*> _rootSections;
    std::map<uint32_t, Section*> _sections;
    uint32_t _counter;
};
}
}
