#include <morphio/mut/soma.h>
#include <morphio/soma.h>

namespace morphio
{
namespace mut
{

Soma::Soma(const morphio::Soma& soma)
{
    _pointProperties =
        Property::PointLevel(soma._properties->_pointLevel, soma._range);
}

} // end namespace mut
} // end namespace morphio
