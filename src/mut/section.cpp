#include <stack>

#include <morphio/mut/section.h>
#include <morphio/mut/morphology.h>

namespace morphio
{
namespace mut
{
Section::Section(Morphology* morphology, int id, SectionType type,
                 const Property::PointLevel& pointProperties)
    : _morphology(morphology)
    , _pointProperties(pointProperties)
    , _id(id)
    , _sectionType(type)
{
}

Section::Section(Morphology* morphology,
                 int id, const morphio::Section& section)
    : Section(morphology, id, section.type(),
              Property::PointLevel(section._properties->_pointLevel,
                                   section._range))
{
}

Section::Section(Morphology* morphology,
                 int id, const Section& section)
    : _morphology(morphology)
    , _pointProperties(section._pointProperties)
    , _id(id)
    , _sectionType(section._sectionType)
{
}


const std::shared_ptr<Section> Section::parent() const {
    return _morphology->_sections.at(_morphology->_parent.at(id()));
}

const bool Section::isRoot() const {
    try {
        parent();
        return false;
    } catch (const std::out_of_range &e) {
        return true;
    }
}

const std::vector<std::shared_ptr<Section>> Section::children() const {
    try {
        return _morphology->_children.at(id());
    } catch (const std::out_of_range &e) {
        return std::vector<std::shared_ptr<Section>>();
    }
}


depth_iterator Section::depth_begin() const
{
    return depth_iterator(*_morphology, const_cast<Section*>(this)->shared_from_this());
}

depth_iterator Section::depth_end() const
{
    return depth_iterator(*_morphology);
}

breadth_iterator Section::breadth_begin() const
{
    return breadth_iterator(*_morphology, const_cast<Section*>(this)->shared_from_this());
}

breadth_iterator Section::breadth_end() const
{
    return breadth_iterator(*_morphology);
}

upstream_iterator Section::upstream_begin() const
{
    return upstream_iterator(*_morphology, const_cast<Section*>(this)->shared_from_this());
}

upstream_iterator Section::upstream_end() const
{
    return upstream_iterator(*_morphology);
}


std::ostream& operator<<(std::ostream& os, Section& section)
{
    os << "id: " << section.id() << std::endl;;
    os << dumpPoints(section.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Section> sectionPtr){
    os << *sectionPtr;
    return os;
}

} // end namespace mut
} // end namespace morphio
