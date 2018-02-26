#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>

#include <functional>


#include <morphio/exceptions.h>
#include <morphio/iterators.h>
#include <morphio/properties.h>
#include <morphio/section.h>
#include <morphio/types.h>
#include <morphio/mut/soma.h>

namespace morphio
{
namespace mut
{


class Morphology
{
public:
    Morphology() : _soma(std::make_shared<Soma>(Soma())) , _counter(0) {}
    Morphology(const morphio::Morphology& morphology);
    virtual ~Morphology();


    const std::set<uint32_t>& rootSections();
    std::map<uint32_t, std::shared_ptr<Section>>& sections();
    std::shared_ptr<Soma> soma();
    void deleteSection(uint32_t id, bool recursive = true);
    uint32_t appendSection(uint32_t parentId, const morphio::Section&, bool recursive = true);
    uint32_t appendSection(uint32_t parentId, SectionType, const Property::PointLevel&);
    void traverse(std::function<void(Morphology& morphology, uint32_t sectionId)>,
                  uint32_t startSection = -1);

    std::shared_ptr<Section> section(uint32_t id){
        return _sections[id];
    }

    uint32_t parent(uint32_t id){
        try {
            return _parent[id];
        } catch (const std::out_of_range &e) {
            return -1;
        }
    }

    std::set<uint32_t> children(uint32_t id){
        try {
            return _children[id];
        } catch (const std::out_of_range &e) {
            return std::set<uint32_t>();
        }
    }


    Property::Properties buildReadOnly();

private:
    friend class Section;

    uint32_t _register(std::shared_ptr<Section>);
    std::shared_ptr<Soma> _soma;
    std::set<uint32_t> _rootSections;
    std::map<uint32_t, std::shared_ptr<Section>> _sections;

    uint32_t _counter;
    std::map<uint32_t, uint32_t> _parent;
    std::map<uint32_t, std::set<uint32_t>> _children;
};

namespace writer
{
void swc(Morphology& morphology);
void asc(Morphology& morphology);
void h5(Morphology& morphology);
} // namespace writer
} // namespace mut
} // namespace morphio
