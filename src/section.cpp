#include <morphio/morphology.h>
#include <morphio/section.h>

namespace morphio
{
Section::Section(const uint32_t id, std::shared_ptr<Property::Properties> properties)
    : _id(id)
    , _properties(properties)
{
    const auto& points = properties->get<Property::Point>();
    const auto& sections = properties->get<Property::Section>();
    if (id >= sections.size())
        LBTHROW(RawDataError("Requested section ID (" + std::to_string(id) +
                             ") is out of array bounds (array size = " +
                             std::to_string(sections.size()) + ")"));

    const size_t start = sections[id][0];
    const size_t end =
        id == sections.size() - 1 ? points.size() : sections[id + 1][0];
    _range = std::make_pair(start, end);

    if (_range.second <= _range.first)
        LBWARN << "Dereferencing broken properties section " << _id << std::endl
               << "Section range: " << _range.first << " -> " << _range.second
               << std::endl;
}

Section::Section(const Section& section)
    : _id(section._id)
    , _range(section._range)
    , _properties(section._properties)
{
}

const Section& Section::operator=(const Section& section)
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
const uint32_t Section::id() const
{
    return _id;
}

const SectionType Section::type() const
{
    auto val = _properties->get<Property::SectionType>()[_id];
    return val;
}

template <typename TProperty>
const gsl::span<const typename TProperty::Type> Section::get() const
{
    auto ptr_start = _properties->get<TProperty>().data() + _range.first;
    return gsl::span<const typename TProperty::Type>(ptr_start, _range.second - _range.first);
}

bool Section::isRoot() const
{
    return _properties->get<Property::Section>()[_id][1] == -1;
}

Section Section::parent() const
{
    if (isRoot())
        LBTHROW("Cannot call Section::parent() on a root node (section id=" +
                std::to_string(_id) + ").");

    const int32_t parent = _properties->get<Property::Section>()[_id][1];
    return Section(parent, _properties);
}

const std::vector<Section> Section::children() const
{
    std::vector<Section> result;
    try
    {
        const std::vector<uint32_t>& children = _properties->children().at(_id);
        result.reserve(children.size());
        for (const uint32_t id : children)
            result.push_back(Section(id, _properties));
        return result;
    }
    catch (const std::out_of_range& oor)
    {
        return result;
    }
}

depth_iterator Section::depth_begin() const
{
    return depth_iterator(*this);
}

depth_iterator Section::depth_end() const
{
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const
{
    return breadth_iterator(*this);
}

breadth_iterator Section::breadth_end() const
{
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const
{
    return upstream_iterator(*this);
}

upstream_iterator Section::upstream_end() const
{
    return upstream_iterator();
}

std::ostream& operator<<(std::ostream& os, const Section& section)
{
    os << section.id();
    return os;
}

const gsl::span<const Point> Section::points() const
{
    return get<Property::Point>();
}
const gsl::span<const float> Section::diameters() const
{
    return get<Property::Diameter>();
}
const gsl::span<const float> Section::perimeters() const
{
    return get<Property::Perimeter>();
}
}
