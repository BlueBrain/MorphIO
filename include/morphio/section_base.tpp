#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/section.h>

namespace morphio {
template <typename T>
SectionBase<T>::SectionBase(const uint32_t id_,
                            const std::shared_ptr<Property::Properties>& properties)
    : _id(id_)
    , _properties(properties) {
    const auto& sections = properties->get<typename T::SectionId>();
    if (_id >= sections.size())
        throw RawDataError(
            "Requested section ID (" + std::to_string(_id) +
            ") is out of array bounds (array size = " + std::to_string(sections.size()) + ")");

    const auto start = static_cast<size_t>(sections[_id][0]);
    const size_t end = _id == sections.size() - 1
                           ? properties->get<typename T::PointAttribute>().size()
                           : static_cast<size_t>(sections[_id + 1][0]);

    _range = std::make_pair(start, end);

    if (_range.second <= _range.first)
        std::cerr << "Dereferencing broken properties section " << _id
                  << "\nSection range: " << _range.first << " -> " << _range.second << '\n';
}

template <typename T>
SectionBase<T>::SectionBase(const SectionBase& other)
    : _id(other._id)
    , _range(other._range)
    , _properties(other._properties) {}

template <typename T>
SectionBase<T>& SectionBase<T>::operator=(const SectionBase& section) {
    if (&section == this)
        return *this;
    _id = section._id;
    _range = section._range;
    _properties = section._properties;
    return *this;
}

template <typename T>
template <typename TProperty>
range<const typename TProperty::Type> SectionBase<T>::get() const {
    const auto& data = _properties->get<TProperty>();
    if (data.empty())
        return {};

    auto ptr_start = data.data() + _range.first;
    return {ptr_start, _range.second - _range.first};
}

template <typename T>
bool SectionBase<T>::isRoot() const {
    return _properties->get<typename T::SectionId>()[_id][1] == -1;
}

template <typename T>
T SectionBase<T>::parent() const {
    if (isRoot())
        throw MissingParentError("Cannot call Section::parent() on a root node (section id=" +
                                 std::to_string(_id) + ").");

    const auto _parent = static_cast<unsigned int>(
        _properties->get<typename T::SectionId>()[_id][1]);
    return {_parent, _properties};
}

template <typename T>
std::vector<T> SectionBase<T>::children() const {
    std::vector<T> result;
    try {
        const std::vector<uint32_t>& _children = _properties->children<typename T::SectionId>().at(
            static_cast<int>(_id));
        result.reserve(_children.size());
        for (const uint32_t id_ : _children)
            result.push_back(T(id_, _properties));

        return result;
    } catch (const std::out_of_range&) {
        return result;
    }
}

}  // namespace morphio
