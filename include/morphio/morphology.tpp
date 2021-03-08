#pragma once

#include <morphio/mut/morphology.h>


TMorphology<SectionT>::TMorphology(const Property::Properties& properties, unsigned int options)
    : _properties(std::make_shared<Property::Properties>(properties)) {
    buildChildren(_properties);

    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());

    // For SWC and ASC, sanitization and modifier application are already taken care of by
    // their respective loaders
    if (properties._cellLevel.fileFormat() == "h5") {
        mut::Morphology mutable_morph(*this);
        mutable_morph.sanitize();
        if (options) {
            mutable_morph.applyModifiers(options);
        }
        _properties = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
        buildChildren(_properties);
    }
}

TMorphology<SectionT>::TMorphology(const HighFive::Group& group, unsigned int options)
    : TMorphology(readers::h5::load(group), options) {}

TMorphology<SectionT>::TMorphology(const std::string& source, unsigned int options)
    : TMorphology(loadURI(source, options), options) {}

TMorphology<SectionT>::TMorphology(mut::Morphology morphology) {
    morphology.sanitize();
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

TMorphology<SectionT>::TMorphology(TMorphology&&) noexcept = default;
TMorphology& TMorphology<SectionT>::operator=(TMorphology&&) noexcept = default;

TMorphology<SectionT>::~TMorphology() = default;

Soma TMorphology<SectionT>::soma() const {
    return Soma(_properties);
}

Mitochondria TMorphology<SectionT>::mitochondria() const {
    return Mitochondria(_properties);
}

const EndoplasmicReticulum TMorphology<SectionT>::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

const std::vector<Property::Annotation>& TMorphology<SectionT>::annotations() const {
    return _properties->_cellLevel._annotations;
}

const std::vector<Property::Marker>& TMorphology<SectionT>::markers() const {
    return _properties->_cellLevel._markers;
}

Section TMorphology<SectionT>::section(uint32_t id) const {
    return {id, _properties};
}

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

template <typename Property>
const std::vector<typename Property::Type>& TMorphology<SectionT>::get() const {
    return _properties->get<Property>();
}

const Points& TMorphology<SectionT>::points() const noexcept {
    return get<Property::Point>();
}

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

const std::vector<morphio::floatType>& TMorphology<SectionT>::diameters() const {
    return get<Property::Diameter>();
}

const std::vector<morphio::floatType>& TMorphology<SectionT>::perimeters() const {
    return get<Property::Perimeter>();
}

const std::vector<SectionType>& TMorphology<SectionT>::sectionTypes() const {
    return get<Property::SectionType>();
}

const CellFamily& TMorphology<SectionT>::cellFamily() const {
    return _properties->cellFamily();
}

const SomaType& TMorphology<SectionT>::somaType() const {
    return _properties->somaType();
}

const std::map<int, std::vector<unsigned int>>& TMorphology<SectionT>::connectivity() const {
    return _properties->children<Property::Section>();
}

const MorphologyVersion& TMorphology<SectionT>::version() const {
    return _properties->version();
}

depth_iterator TMorphology<SectionT>::depth_begin() const {
    return depth_iterator(*this);
}

depth_iterator TMorphology<SectionT>::depth_end() const {
    return depth_iterator();
}

breadth_iterator TMorphology<SectionT>::breadth_begin() const {
    return breadth_iterator(*this);
}

breadth_iterator TMorphology<SectionT>::breadth_end() const {
    return breadth_iterator();
}