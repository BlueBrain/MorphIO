/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iostream>

#include <morphio/vasc/section.h>

#include "../point_utils.h"

namespace morphio {
namespace vasculature {

using graph_iterator = graph_iterator_t<Section, Vasculature>;

Section::Section(uint32_t id, const std::shared_ptr<property::Properties>& properties)
    : id_(id)
    , properties_(properties) {
    const auto& sections = properties->get<property::VascSection>();
    if (id_ >= sections.size()) {
        throw RawDataError(
            "Requested section ID (" + std::to_string(id_) +
            ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")");
    }
    const size_t start = sections[id_];
    const size_t end = id_ == sections.size() - 1 ? properties->get<property::Point>().size()
                                                  : sections[id_ + 1];
    range_ = std::make_pair(start, end);

    if (range_.second <= range_.first) {
        // TODO: shouldn't print to std::cerr
        std::cerr << "Dereferencing broken properties section " << id_
                  << "\nSection range: " << range_.first << " -> " << range_.second << '\n';
    }
}

Section& Section::operator=(const Section& section) {
    if (&section == this) {
        return *this;
    }
    id_ = section.id_;
    range_ = section.range_;
    properties_ = section.properties_;
    return *this;
}

bool Section::operator==(const Section& other) const {
    return other.id_ == id_ && other.properties_ == properties_;
}

bool Section::operator!=(const Section& other) const {
    return !(*this == other);
}

uint32_t Section::id() const noexcept {
    return id_;
}

template <typename TProperty>
range<const typename TProperty::Type> Section::get() const {
    const auto& data = properties_->get<TProperty>();
    if (data.empty()) {
        return range<const typename TProperty::Type>();
    }
    auto ptr_start = data.data() + range_.first;
    return range<const typename TProperty::Type>(ptr_start, range_.second - range_.first);
}

std::vector<Section> Section::predecessors() const {
    std::vector<Section> result;
    const auto it = properties_->predecessors().find(id_);
    if (it != properties_->predecessors().end()) {
        result.reserve(it->second.size());
        for (uint32_t id : it->second) {
            result.emplace_back(id, properties_);
        }
    }
    return result;
}

std::vector<Section> Section::successors() const {
    std::vector<Section> result;
    const auto it = properties_->successors().find(id_);
    if (it != properties_->successors().end()) {
        result.reserve(it->second.size());
        for (uint32_t id : it->second) {
            result.emplace_back(id, properties_);
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
    return properties_->get<property::SectionType>()[id_];
}

floatType Section::length() const {
    const auto& points_ = this->points();
    if (points_.size() < 2) {
        return 0;
    }

    size_t last = points_.size() - 1;
    return euclidean_distance(points_[0], points_[last]);
}

range<const Point> Section::points() const {
    return get<property::Point>();
}

range<const floatType> Section::diameters() const {
    return get<property::Diameter>();
}

bool Section::operator<(const Section& other) const {
    return this->id_ > other.id();
}

graph_iterator Section::begin() const {
    return graph_iterator(*this);
}

graph_iterator Section::end() const {
    return graph_iterator();
}
}  // namespace vasculature
}  // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::vasculature::Section& section) {
    const auto& points = section.points();
    if (points.empty()) {
        os << "Section(id=" << section.id() << ", points=[])";
    } else {
        os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., ("
           << points[points.size() - 1] << ")])";
    }
    return os;
}
