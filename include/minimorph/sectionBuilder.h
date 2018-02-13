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
    SectionType& type(){ return _somaType; }
    std::vector<Point>& points(){ return _pointProperties._points;}
    std::vector<float> diameters(){ return _pointProperties._diameters;}

private:
    Property::PointLevel _pointProperties;
    SectionType _somaType;
};

class Section
{
public:
    Section* const parent();
    const std::set<Section*>& children();
    const uint32_t id() const { return _id; }
    SectionType& type() { return _sectionType; }
    std::vector<Point>& points(){ return _pointProperties._points;}
    std::vector<float> diameters(){ return _pointProperties._diameters;}
    std::vector<float> perimeters(){ return _pointProperties._perimeters;}

private:
    void traverse(Morphology* morphology,
                  std::function<void(Morphology* morphology, Section* section)>);
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
    virtual ~Morphology();
    const std::set<Section*>& rootSections();

    Soma& soma();
    void deleteSection(Section*, bool recursive = true);
    uint32_t appendSection(Section* parent, const minimorph::Section&, bool recursive=true);
    uint32_t appendSection(Section* parent, SectionType, const Property::PointLevel&);
    void traverse(std::function<void(Morphology* morphology, Section* section)>,
                  Section* rootSection = nullptr);

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
void h5(Morphology& morphology);
void swc(Morphology& morphology);
void asc(Morphology& morphology);
}

}
}
