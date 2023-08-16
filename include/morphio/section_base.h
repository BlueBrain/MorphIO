#pragma once

#include <cstdint>  // uint32_t
#include <memory>   // std::shared_ptr
#include <vector>   // std::vector

#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/types.h>
#include <morphio/vector_types.h>

namespace morphio {
/**
   This CRTP
 (https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) defines
 basic methods that every kind of sections (should them be neuronal or
 mithochondrial) must define.

   The CRTP is used here so that the methods of the base class can return object
 of the derived class. Examples: T SectionBase::parent() std::vector<T>
 SectionBase::children()
 **/

template <typename T>
class SectionBase
{
  public:
    inline bool operator==(const SectionBase& other) const noexcept;
    inline bool operator!=(const SectionBase& other) const noexcept;

    /**
     * Return true if this section is a root section (parent ID == -1)
     **/
    bool isRoot() const;

    /**
     * Return the parent section of this section
     *
     * @throw MissingParentError is the section doesn't have a parent.
     */
    T parent() const;

    /**
     * Return a list of children sections
     */
    std::vector<T> children() const;

    /** Return the ID of this section. */
    uint32_t id() const noexcept { return id_; }

  protected:
    SectionBase(uint32_t id, const std::shared_ptr<Property::Properties>& properties);

    template <typename Property>
    range<const typename Property::Type> get() const;

    uint32_t id_ = 0;
    SectionRange range_;
    std::shared_ptr<Property::Properties> properties_;
};

template <typename T>
SectionBase<T>::SectionBase(uint32_t id, const std::shared_ptr<Property::Properties>& properties)
    : id_(id)
    , properties_(properties) {
    const auto& sections = properties->get<typename T::SectionId>();
    if (id_ >= sections.size()) {
        throw RawDataError(
            "Requested section ID (" + std::to_string(id_) +
            ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")");
    }

    const auto start = static_cast<size_t>(sections[id_][0]);
    const size_t end = id_ == sections.size() - 1
                           ? properties->get<typename T::PointAttribute>().size()
                           : static_cast<size_t>(sections[id_ + 1][0]);

    range_ = std::make_pair(start, end);

    if (range_.second <= range_.first) {
        std::cerr << "Dereferencing broken properties section " << id_
                  << "\nSection range: " << range_.first << " -> " << range_.second << '\n';
    }
}

template <typename T>
inline bool SectionBase<T>::operator==(const SectionBase& other) const noexcept {
    return other.id_ == id_ && other.properties_ == properties_;
}

template <typename T>
inline bool SectionBase<T>::operator!=(const SectionBase& other) const noexcept {
    return !(*this == other);
}

template <typename T>
template <typename TProperty>
range<const typename TProperty::Type> SectionBase<T>::get() const {
    const auto& data = properties_->get<TProperty>();
    if (data.empty()) {
        return {};
    }

    auto ptr_start = data.data() + range_.first;
    return {ptr_start, range_.second - range_.first};
}

template <typename T>
bool SectionBase<T>::isRoot() const {
    return properties_->get<typename T::SectionId>()[id_][1] == -1;
}

template <typename T>
T SectionBase<T>::parent() const {
    if (isRoot()) {
        throw MissingParentError("Cannot call Section::parent() on a root node (section id=" +
                                 std::to_string(id_) + ").");
    }

    const auto _parent = static_cast<unsigned int>(
        properties_->get<typename T::SectionId>()[id_][1]);
    return {_parent, properties_};
}

template <typename T>
std::vector<T> SectionBase<T>::children() const {
    const auto& section_children = properties_->children<typename T::SectionId>();

    if (section_children.empty()) {
        return {};
    }

    const auto it = section_children.find(static_cast<int>(id_));
    if (it == section_children.end()) {
        return {};
    }

    std::vector<T> result;
    const std::vector<uint32_t> children = it->second;
    result.reserve(children.size());
    for (uint32_t id : children) {
        result.push_back(T(id, properties_));
    }

    return result;
}


}  // namespace morphio
