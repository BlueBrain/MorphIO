#include <minimorph/soma.h>
#include <minimorph/section.h>

namespace minimorph
{
Soma::Soma(PropertiesPtr properties)
    : _properties(properties)
{
    uint32_t id = 0;
    const auto& points = properties->get<Property::Point>();
    const auto& sections = properties->get<Property::Section>();
    if(id >= sections.size())
        LBTHROW(RawDataError("Requested section ID (" + std::to_string(id) + \
                             ") is out of array bounds (array size = " + \
                             std::to_string(sections.size()) + ")"));

    const size_t start = sections[id][0];
    const size_t end = id == sections.size() - 1
        ? points.size()
        : sections[id + 1][0];
    _range = std::make_pair(start, end);

    if (_range.second <= _range.first)
        LBWARN << "Dereferencing broken soma "
               << "Section range: " << _range.first << " -> " << _range.second
               << std::endl;
}

const std::vector<Section> Soma::rootSections() const
{
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& children = _properties->children().at(0);
        result.reserve(children.size());
        for (const uint32_t id : children)
            result.push_back(Section(id, _properties));
        return result;
    }
    catch (const std::out_of_range& oor) {
        return result;
    }
}

const SectionType Soma::type() const {
    return get<Property::SectionType>()[0];
}

template <typename TProperty> const gsl::span<const typename TProperty::Type> Soma::get() const
{
    auto ptr_start = _properties->get<TProperty>().data() + _range.first;
    return gsl::span<const typename TProperty::Type>(ptr_start, _range.second);
}

const Point Soma::somaCenter() const
{
    auto points = get<Property::Point>();
    float x = 0, y = 0, z = 0;
    float size = float(points.size());
    for (const Point& point : points)
    {
        x += point[0];
        y += point[1];
        z += point[2];
    }
    return Point({x/size, y/size, z/size});
}

template const gsl::span<const Property::Point::Type> Soma::get<Property::Point>() const;
template const gsl::span<const Property::Diameter::Type> Soma::get<Property::Diameter>() const;
}
