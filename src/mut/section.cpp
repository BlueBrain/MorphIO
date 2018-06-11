#include <stack>

#include <morphio/mut/section.h>
#include <morphio/mut/morphology.h>

namespace morphio
{
namespace mut
{
Section::Section(int id, SectionType type,
                 const Property::PointLevel& pointProperties)
    : _pointProperties(pointProperties)
    , _id(id)
    , _sectionType(type)
{
}

Section::Section(const morphio::Section& section)
    : Section(section.id(), section.type(),
              Property::PointLevel(section._properties->_pointLevel,
                                   section._range))
{
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
