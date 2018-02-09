#include "minimorph/section.h"
#include "minimorph/morphology.h"

namespace minimorph
{

Section::Section(const uint32_t id, PropertiesPtr properties)
    : _id(id)
    , _properties(properties)
{
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
        LBWARN << "Dereferencing broken properties section " << _id << std:: endl
               << "Section range: " << _range.first << " -> " << _range.second
               << std::endl;
}

Section::Section(const Section& section)
    : _id(section._id)
    , _range(section._range)
    , _properties(section._properties)
{
}

Section& Section::operator=(const Section& section)
{
    if (&section == this)
        return *this;
    _id = section._id;
    _range = section._range;
    _properties = section._properties;
    return *this;
}

bool Section::operator==(const Section& other) const
{
    return other._id == _id && other._properties == _properties;
}

bool Section::operator!=(const Section& other) const
{
    return !(*this == other);
}
uint32_t Section::getID() const
{
    return _id;
}

const SectionType Section::getType() const
{
    return get<Property::SectionType>()[_id];
}


template <typename TProperty> const gsl::span<const typename TProperty::Type> Section::get() const
{
    auto ptr_start = _properties->get<TProperty>().data() + _range.first;
    return gsl::span<const typename TProperty::Type>(ptr_start, _range.second);
}

std::shared_ptr<Section> Section::getParent() const
{
    const int32_t parent = _properties->get<Property::Section>()[_id][1];
    return (parent > -1) ? std::make_shared<Section>(Section(parent, _properties)) : nullptr;
}

Sections Section::getChildren() const
{
    Sections result;
    try {
        const uint32_ts& children = _properties->getChildren().at(_id);
        result.reserve(children.size());
        for (const uint32_t id : children)
            result.push_back(Section(id, _properties));
        return result;
    }
    catch (const std::out_of_range& oor) {
        return result;
    }
}

depth_iterator Section::depth_begin() {
    return depth_iterator(*this);
}

depth_iterator Section::depth_end() {
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() {
    return breadth_iterator(*this);
}

breadth_iterator Section::breadth_end() {
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() {
    return upstream_iterator(*this);
}

upstream_iterator Section::upstream_end() {
    return upstream_iterator();
}


std::ostream& operator<<(std::ostream& os, const Section& section){
    os << section.getID();
    return os;
}

const gsl::span<const Point> Section::getPoints() const { return get<Property::Point>(); }
const gsl::span<const float> Section::getDiameters() const { return get<Property::Diameter>(); }
const gsl::span<const float> Section::getPerimeters() const { return get<Property::Perimeter>(); }

}
