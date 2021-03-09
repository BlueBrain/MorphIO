#pragma once

#include <morphio/mut/morphology.h>
//#include "../src/readers/morphologyHDF5.h"

namespace morphio
{
    namespace readers
    {
        namespace h5
        {
            Property::Properties load(const std::string& uri);
            Property::Properties load(const HighFive::Group& group);
        }
    }
}



namespace morphio {

class Section;

template <typename SectionT>
TMorphology<SectionT>::TMorphology(const Property::Properties& properties, unsigned int options)
    : _properties(std::make_shared<Property::Properties>(properties)) {
    buildChildren(_properties);

    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());

    // For SWC and ASC, sanitization and modifier application are already taken care of by
    // their respective loaders
    if (properties._cellLevel.fileFormat() == "h5") {
        mut::TMorphology<SectionT> mutable_morph(*this);
        mutable_morph.sanitize();
        if (options) {
            mutable_morph.applyModifiers(options);
        }
        _properties = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
        buildChildren(_properties);
    }
}
template <typename SectionT>
TMorphology<SectionT>::TMorphology(const HighFive::Group& group, unsigned int options)
    : TMorphology(readers::h5::load(group), options) {}

template <typename SectionT>
TMorphology<SectionT>::TMorphology(const std::string& source, unsigned int options)
    : TMorphology(loadURI(source, options), options) {}

template <typename SectionT>
TMorphology<SectionT>::TMorphology(mut::Morphology morphology) {
    morphology.sanitize();
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

template <typename SectionT>
TMorphology<SectionT>::TMorphology(TMorphology&&) noexcept = default;

template <typename SectionT>
TMorphology& TMorphology<SectionT>::operator=(TMorphology&&) noexcept = default;

template <typename SectionT>
TMorphology<SectionT>::~TMorphology() = default;

template <typename SectionT>
Soma TMorphology<SectionT>::soma() const {
    return Soma(_properties);
}

template <typename SectionT>
Mitochondria TMorphology<SectionT>::mitochondria() const {
    return Mitochondria(_properties);
}

template <typename SectionT>
const EndoplasmicReticulum TMorphology<SectionT>::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

template <typename SectionT>
const std::vector<Property::Annotation>& TMorphology<SectionT>::annotations() const {
    return _properties->_cellLevel._annotations;
}

template <typename SectionT>
const std::vector<Property::Marker>& TMorphology<SectionT>::markers() const {
    return _properties->_cellLevel._markers;
}

template <typename SectionT>
Section TMorphology<SectionT>::section(uint32_t id) const {
    return {id, _properties};
}

template <typename SectionT>
std::vector<Section> TMorphology<SectionT>::rootSections() const {
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& children =
            _properties->children<morphio::Property::Section>().at(-1);
        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }

        return result;
    } catch (const std::out_of_range&) {
        return result;
    }
}

template <typename SectionT>
std::vector<Section> TMorphology<SectionT>::sections() const {
    // TODO: Make this more performant when needed
    std::vector<Section> sections_;
    auto count = _properties->get<morphio::Property::Section>().size();
    sections_.reserve(count);
    for (unsigned int i = 0; i < count; ++i) {
        sections_.emplace_back(section(i));
    }
    return sections_;
}

template <typename Property, SectionT>
const std::vector<typename Property::Type>& TMorphology<SectionT>::get() const {
    return _properties->get<Property>();
}

template <typename SectionT>
const Points& TMorphology<SectionT>::points() const noexcept {
    return get<Property::Point>();
}

template <typename SectionT>
std::vector<uint32_t> TMorphology<SectionT>::sectionOffsets() const {
    const std::vector<Property::Section::Type>& indices_and_parents = get<Property::Section>();
    auto size = indices_and_parents.size();
    std::vector<uint32_t> indices(size + 1);
    std::transform(indices_and_parents.begin(),
                   indices_and_parents.end(),
                   indices.begin(),
                   [](const Property::Section::Type& pair) { return pair[0]; });
    indices[size] = static_cast<uint32_t>(points().size());
    return indices;
}

template <typename SectionT>
const std::vector<morphio::floatType>& TMorphology<SectionT>::diameters() const {
    return get<Property::Diameter>();
}

template <typename SectionT>
const std::vector<morphio::floatType>& TMorphology<SectionT>::perimeters() const {
    return get<Property::Perimeter>();
}

template <typename SectionT>
const std::vector<SectionType>& TMorphology<SectionT>::sectionTypes() const {
    return get<Property::SectionType>();
}

template <typename SectionT>
const CellFamily& TMorphology<SectionT>::cellFamily() const {
    return _properties->cellFamily();
}

template <typename SectionT>
const SomaType& TMorphology<SectionT>::somaType() const {
    return _properties->somaType();
}

template <typename SectionT>
const std::map<int, std::vector<unsigned int>>& TMorphology<SectionT>::connectivity() const {
    return _properties->children<Property::Section>();
}

template <typename SectionT>
const MorphologyVersion& TMorphology<SectionT>::version() const {
    return _properties->version();
}

template <typename SectionT>
depth_iterator TMorphology<SectionT>::depth_begin() const {
    return depth_iterator(*this);
}

template <typename SectionT>
depth_iterator TMorphology<SectionT>::depth_end() const {
    return depth_iterator();
}

template <typename SectionT>
breadth_iterator TMorphology<SectionT>::breadth_begin() const {
    return breadth_iterator(*this);
}

template <typename SectionT>
breadth_iterator TMorphology<SectionT>::breadth_end() const {
    return breadth_iterator();
}

}  // namespace morphio
