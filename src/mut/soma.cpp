#include <morphio/mut/soma.h>
#include <morphio/soma.h>

#include <morphio/section.h>

namespace morphio
{
namespace mut
{

Soma::Soma(const Property::PointLevel &pointProperties) : _pointProperties(pointProperties)
{
}

Soma::Soma(const morphio::Soma& soma)
{
    _pointProperties =
        Property::PointLevel(soma._properties->_pointLevel, soma._range);
}

std::ostream& operator<<(std::ostream& os, Soma& soma)
{
    os << dumpPoints(soma.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Soma> somaPtr){
    os << *somaPtr;
    return os;
}

} // end namespace mut
} // end namespace morphio
