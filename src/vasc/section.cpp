#include <morphio/iterators.h>
#include <morphio/vasc/section.h>

namespace morphio {
namespace vasculature {
Section::Section(const uint32_t id_,
    std::shared_ptr<property::Properties> properties)
    : _id(id_)
    , _properties(properties)
{
    const auto& sections = properties->get<property::VascSection>();
    if (id_ >= sections.size())
        LBTHROW(RawDataError("Requested section ID (" + std::to_string(id_) + ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")"));
    const size_t start = sections[id_];
    const size_t end_ = id_ == sections.size() - 1
                            ? properties->get<property::Point>().size()
                            : sections[id_ + 1];
    _range = std::make_pair(start, end_);

    if (_range.second <= _range.first)
        LBWARN << "Dereferencing broken properties section " << _id << std::endl
               << "Section range: " << _range.first << " -> " << _range.second << std::endl;
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
        const std::vector<uint32_t>& predecessors_ = _properties->predecessors().at(_id);
        result.reserve(predecessors_.size());
        for (const uint32_t id_ : predecessors_)
            result.push_back(Section(id_, _properties));
        return result;
    } catch (...){
        return result;
    }
}

const std::vector<Section> Section::successors() const
{
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& successors_ = _properties->successors().at(_id);
        result.reserve(successors_.size());
        for (const uint32_t id_ : successors_)
            result.push_back(Section(id_, _properties));
        return result;
    } catch (...) {
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
    auto points_ = this->points();
    if (points_.size() < 2)
        return 0;

    size_t last = points_.size() - 1;
    return distance(points_[0], points_[last]);
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
