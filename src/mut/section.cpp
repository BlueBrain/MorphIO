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


void Section::traverse(
    Morphology& morphology,
    std::function<void(Morphology& morphology, uint32_t section)> fun)
{
    // depth first traversal
    std::vector<uint32_t> stack;
    stack.push_back(_id);
    while (!stack.empty())
    {
        uint32_t parent = stack.back();
        stack.pop_back();
        fun(morphology, parent);

        for (auto child : morphology.children(_id))
        {
            stack.push_back(child);
        }
    }
}
} // end namespace mut
} // end namespace morphio
