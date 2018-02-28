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


void Section::traverse(
    Morphology& morphology,
    std::function<void(Morphology& morphology, uint32_t section)> fun)
{
    // depth first traversal
    std::stack<uint32_t> stack;
    stack.push(_id);
    while (!stack.empty())
    {
        uint32_t node_id = stack.top();
        stack.pop();
        fun(morphology, node_id);

        auto &parent = morphology.children(node_id);
        for (auto it = parent.rbegin(); it != parent.rend(); ++it)
        {
            stack.push(*it);
        }
    }
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
