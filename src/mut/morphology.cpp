#include "morphio/mut/morphology.h"

namespace morphio {
namespace mut {
void _appendProperties(Property::PointLevel& to, const Property::PointLevel& from, int offset = 0) {
    _appendVector(to._points, from._points, offset);
    _appendVector(to._diameters, from._diameters, offset);

    if (!from._perimeters.empty())
        _appendVector(to._perimeters, from._perimeters, offset);
}


/**
   Return false if there is no duplicate point
**/
bool _checkDuplicatePoint(const std::shared_ptr<Section>& parent,
                          const std::shared_ptr<Section>& current) {
    // Weird edge case where parent is empty: skipping it
    if (parent->points().empty())
        return true;

    if (current->points().empty())
        return false;

    if (parent->points().back() != current->points().front())
        return false;

    // // As perimeter is optional, it must either be defined for parent and
    // current
    // // or not be defined at all
    // if(parent->perimeters().empty() != current->perimeters().empty())
    //     return false;

    // if(!parent->perimeters().empty() &&
    //    parent->perimeters()[parent->perimeters().size()-1] !=
    //    current->perimeters()[0])
    //     return false;

    return true;
}
}  // namespace mut
}  // namespace morphio
