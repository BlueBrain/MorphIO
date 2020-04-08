#include <morphio/vasc/section.h>

namespace morphio {
namespace vasculature {

using graph_iterator = graph_iterator_t<Section, Vasculature>;

Section::Section(const uint32_t id_, const std::shared_ptr<property::Properties>& properties)
    : _id(id_)
    , _properties(properties) {
    const auto& sections = properties->get<property::VascSection>();
    if (id_ >= sections.size()) {
        throw RawDataError(
            "Requested section ID (" + std::to_string(id_) +
            ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")");
    }
    const size_t start = sections[id_];
    const size_t end_ = id_ == sections.size() - 1 ? properties->get<property::Point>().size()
                                                   : sections[id_ + 1];
    _range = std::make_pair(start, end_);

    if (_range.second <= _range.first)
        std::cerr << "Dereferencing broken properties section " << _id
                  << "\nSection range: " << _range.first << " -> " << _range.second << '\n';
}

Section& Section::operator=(const Section& section) {
    if (&section == this)
        return *this;
    _id = section._id;
    _range = section._range;
    _properties = section._properties;
    return *this;
}

bool Section::operator==(const Section& other) const {
    return other._id == _id && other._properties == _properties;
}

bool Section::operator!=(const Section& other) const {
    return !(*this == other);
}

uint32_t Section::id() const noexcept {
    return _id;
}

template <typename TProperty>
range<const typename TProperty::Type> Section::get() const {
    auto& data = _properties->get<TProperty>();
    if (data.empty()) {
        return range<const typename TProperty::Type>();
    }
    auto ptr_start = data.data() + _range.first;
    return range<const typename TProperty::Type>(ptr_start, _range.second - _range.first);
}

std::vector<Section> Section::predecessors() const {
    std::vector<Section> result;
    const auto it = _properties->predecessors().find(_id);
    if (it != _properties->predecessors().end()) {
        result.reserve(it->second.size());
        for (const uint32_t id_ : it->second) {
            result.emplace_back(id_, _properties);
        }
    }
    return result;
}

std::vector<Section> Section::successors() const {
    std::vector<Section> result;
    const auto it = _properties->successors().find(_id);
    if (it != _properties->successors().end()) {
        result.reserve(it->second.size());
        for (const uint32_t id_ : it->second) {
            result.emplace_back(id_, _properties);
        }
    }
    return result;
}

std::vector<Section> Section::neighbors() const {
    auto pre = this->predecessors();
    const auto& suc = this->successors();
    pre.reserve(pre.size() + suc.size());
    std::copy(suc.begin(), suc.end(), std::back_inserter(pre));
    return pre;
}

VascularSectionType Section::type() const {
    auto val = _properties->get<property::SectionType>()[_id];
    return val;
}

floatType Section::length() const {
    const auto& points_ = this->points();
    if (points_.size() < 2)
        return 0;

    size_t last = points_.size() - 1;
    return distance(points_[0], points_[last]);
}

range<const Point> Section::points() const {
    return get<property::Point>();
}

range<const floatType> Section::diameters() const {
    return get<property::Diameter>();
}

bool Section::operator<(const Section& other) const {
    return this->_id > other.id();
}

graph_iterator Section::begin() const {
    return graph_iterator(*this);
}

graph_iterator Section::end() const {
    return graph_iterator();
}
}  // namespace vasculature
}  // namespace morphio
