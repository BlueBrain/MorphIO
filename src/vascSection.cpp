#include <morphio/vascMorphology.h>
#include <morphio/vascSection.h>
#include <morphio/vector_types.h>

namespace morphio {
VasculatureSection::VasculatureSection(const uint32_t id,
        std::shared_ptr<VasculatureProperty::Properties> properties)
        : _id(id)
        , _properties(properties)
{
    const auto& sections = properties->get<VasculatureSection::SectionId>();
    if (id >= sections.size())
        LBTHROW(RawDataError("Requested section ID (" + std::to_string(id) + ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")"));
    const size_t start = sections[id];
    const size_t end = id == sections.size() - 1
                            ? properties->get<VasculatureSection::PointAttribute>().size()
                            : sections[id + 1];
    _range = std::make_pair(start, end);

    if (_range.second <= _range.first)
        LBWARN << "Dereferencing broken properties section " << _id << std::endl
        << "Section range: " << _range.first << " -> " << _range.second << std::endl;
}

VasculatureSection::VasculatureSection(const VasculatureSection &section) :_id(section._id), _range(section._range), _properties(section._properties)
{}

const VasculatureSection& VasculatureSection::operator=(const VasculatureSection& section)
{
    if (&section == this)
        return *this;
    _id = section._id;
    _range = section._range;
    _properties = section._properties;
    return *this;
}

bool VasculatureSection::operator==(const VasculatureSection& other) const
{
    return other._id == _id && other._properties == _properties;
}

bool VasculatureSection::operator!=(const VasculatureSection& other) const
{
    return !(*this == other);
}

uint32_t VasculatureSection::id() const
{
    return _id;
}

template <typename TProperty>
const range<const typename TProperty::Type> VasculatureSection::get() const
{
    auto& data = _properties->get<TProperty>();
    if (data.empty())
        return range<const typename TProperty::Type>();
    auto ptr_start = data.data() + _range.first;
    return range<const typename TProperty::Type>(ptr_start,
            _range.second - _range.first);
}

const std::vector<VasculatureSection> VasculatureSection::predecessors() const
{
    std::vector<VasculatureSection> result;
    try {
        const std::vector<uint32_t>& predecessors = _properties->predecessors<VasculatureSection::SectionId>().at(_id);
        result.reserve(predecessors.size());
        for (const uint32_t id : predecessors)
            result.push_back(VasculatureSection(id, _properties));
        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}

const std::vector<VasculatureSection> VasculatureSection::successors() const
{
    std::vector<VasculatureSection> result;
    try {
        const std::vector<uint32_t>& successors = _properties->successors<VasculatureSection::SectionId>().at(_id);
        result.reserve(successors.size());
        for (const uint32_t id : successors)
            result.push_back(VasculatureSection(id, _properties));
        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}

const std::vector<VasculatureSection> VasculatureSection::neighbors() const
{
    std::vector<VasculatureSection> pre = this->predecessors();
    std::vector<VasculatureSection> suc = this->successors();
    for (size_t i = 0; i < suc.size(); ++i) {
        pre.push_back(suc[i]);
    }
    return pre;
}

SectionType VasculatureSection::type() const
{
    auto val = _properties->get<VasculatureProperty::SectionType>()[_id];
    return val;
}

float VasculatureSection::length() const
{
    auto points = this->points();
    int last = points.size() - 1;
    if (last < 1)
        return 0;
    return distance(points[0], points[last]);
}

const range<const Point> VasculatureSection::points() const
{
    return get<VasculatureProperty::Point>();
}

const range<const float> VasculatureSection::diameters() const
{
    return get<VasculatureProperty::Diameter>();
}

bool VasculatureSection::operator<(const VasculatureSection& other) const
{
    if (this->_id > other.id())
        return true;
    return false;
}

graph_iterator VasculatureSection::begin() const
{
    return graph_iterator(*this);
}

graph_iterator VasculatureSection::end() const
{
    return graph_iterator();
}

}