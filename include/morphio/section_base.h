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
    inline uint32_t id() const noexcept;

  protected:
    SectionBase(uint32_t id, const std::shared_ptr<Property::Properties>& properties);

    template <typename Property>
    range<const typename Property::Type> get() const;

    uint32_t _id = 0;
    SectionRange _range;
    std::shared_ptr<Property::Properties> properties_;
};

template <typename T>
inline bool SectionBase<T>::operator==(const SectionBase& other) const noexcept {
    return other._id == _id && other.properties_ == properties_;
}

template <typename T>
inline bool SectionBase<T>::operator!=(const SectionBase& other) const noexcept {
    return !(*this == other);
}

template <typename T>
inline uint32_t SectionBase<T>::id() const noexcept {
    return _id;
}

template <typename T>
SectionBase<T>::SectionBase(const uint32_t id_,
                            const std::shared_ptr<Property::Properties>& properties)
    : _id(id_)
    , properties_(properties) {
    const auto& sections = properties->get<typename T::SectionId>();
    if (_id >= sections.size()) {
        throw RawDataError(
            "Requested section ID (" + std::to_string(_id) +
            ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")");
    }

    const auto start = static_cast<size_t>(sections[_id][0]);
    const size_t end = _id == sections.size() - 1
                           ? properties->get<typename T::PointAttribute>().size()
                           : static_cast<size_t>(sections[_id + 1][0]);

    _range = std::make_pair(start, end);

    if (_range.second <= _range.first) {
        std::cerr << "Dereferencing broken properties section " << _id
                  << "\nSection range: " << _range.first << " -> " << _range.second << '\n';
    }
}

template <typename T>
template <typename TProperty>
range<const typename TProperty::Type> SectionBase<T>::get() const {
    const auto& data = properties_->get<TProperty>();
    if (data.empty()) {
        return {};
    }

    auto ptr_start = data.data() + _range.first;
    return {ptr_start, _range.second - _range.first};
}

template <typename T>
bool SectionBase<T>::isRoot() const {
    return properties_->get<typename T::SectionId>()[_id][1] == -1;
}

template <typename T>
T SectionBase<T>::parent() const {
    if (isRoot()) {
        throw MissingParentError("Cannot call Section::parent() on a root node (section id=" +
                                 std::to_string(_id) + ").");
    }

    const auto _parent = static_cast<unsigned int>(
        properties_->get<typename T::SectionId>()[_id][1]);
    return {_parent, properties_};
}

template <typename T>
std::vector<T> SectionBase<T>::children() const {
    std::vector<T> result;
    try {
        const std::vector<uint32_t>& _children = properties_->children<typename T::SectionId>().at(
            static_cast<int>(_id));
        result.reserve(_children.size());
        for (const uint32_t id_ : _children) {
            result.push_back(T(id_, properties_));
        }

        return result;
    } catch (const std::out_of_range&) {
        return result;
    }
}


}  // namespace morphio
