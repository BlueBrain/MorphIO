#include <morphio/vasc/section.h>
#include <morphio/iterators.h>

namespace morphio {
namespace vasculature {
Section::Section(const uint32_t id,
        std::shared_ptr<property::Properties> properties)
        : _id(id)
        , _properties(properties)
{
    const auto& sections = properties->get<Section::SectionId>();
    if (id >= sections.size())
        LBTHROW(RawDataError("Requested section ID (" + std::to_string(id) + ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")"));
    const size_t start = sections[id];
    const size_t end = id == sections.size() - 1
                            ? properties->get<Section::PointAttribute>().size()
                            : sections[id + 1];
    _range = std::make_pair(start, end);

    if (_range.second <= _range.first)
        LBWARN << "Dereferencing broken properties section " << _id << std::endl
        << "Section range: " << _range.first << " -> " << _range.second << std::endl;
}

Section::Section(const Section &section) :_id(section._id), _range(section._range), _properties(section._properties)
{}

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

uint32_t Section::id() const
{
    return _id;
}

template <typename TProperty>
const range<const typename TProperty::Type> Section::get() const
{
    auto& data = _properties->get<TProperty>();
    if (data.empty())
        return range<const typename TProperty::Type>();
    auto ptr_start = data.data() + _range.first;
    return range<const typename TProperty::Type>(ptr_start,
            _range.second - _range.first);
}

const std::vector<Section> Section::predecessors() const
{
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& predecessors = _properties->predecessors<Section::SectionId>().at(_id);
        result.reserve(predecessors.size());
        for (const uint32_t id : predecessors)
            result.push_back(Section(id, _properties));
        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}

const std::vector<Section> Section::successors() const
{
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& successors = _properties->successors<Section::SectionId>().at(_id);
        result.reserve(successors.size());
        for (const uint32_t id : successors)
            result.push_back(Section(id, _properties));
        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}

const std::vector<Section> Section::neighbors() const
{
    std::vector<Section> pre = this->predecessors();
    std::vector<Section> suc = this->successors();
    for (size_t i = 0; i < suc.size(); ++i) {
        pre.push_back(suc[i]);
    }
    return pre;
}

VascularSectionType Section::type() const
{
    auto val = _properties->get<property::SectionType>()[_id];
    return val;
}

float Section::length() const
{
    auto points = this->points();
    int last = points.size() - 1;
    if (last < 1)
        return 0;
    return distance(points[0], points[last]);
}

const range<const Point> Section::points() const
{
    return get<property::Point>();
}

const range<const float> Section::diameters() const
{
    return get<property::Diameter>();
}

bool Section::operator<(const Section& other) const
{
    return this->_id > other.id();
}

graph_iterator Section::begin() const
{
    return graph_iterator(*this);
}

graph_iterator Section::end() const
{
    return graph_iterator();
}

}
}