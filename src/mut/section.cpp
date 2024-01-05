/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>  // any_of

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/vector_types.h>
#include <morphio/warning_handling.h>

#include "../point_utils.h"

namespace morphio {
namespace mut {

Section::Section(Morphology* morphology,
                 unsigned int id,
                 SectionType type,
                 const Property::PointLevel& pointProperties)
    : morphology_(morphology)
    , point_properties_(pointProperties)
    , id_(id)
    , section_type_(type) {}

Section::Section(Morphology* morphology, unsigned int id, const morphio::Section& section)
    : Section(morphology,
              id,
              section.type(),
              Property::PointLevel(section.properties_->_pointLevel, section.range_)) {}

Section::Section(Morphology* morphology, unsigned int id, const Section& section)
    : morphology_(morphology)
    , point_properties_(section.point_properties_)
    , id_(id)
    , section_type_(section.section_type_) {}

void Section::throwIfNoOwningMorphology() const {
    if (morphology_ == nullptr) {
        throw std::runtime_error("Section does not belong to a morphology, impossible operation");
    }
}

Morphology* Section::getOwningMorphologyOrThrow() const {
    throwIfNoOwningMorphology();
    return morphology_;
}

const std::shared_ptr<Section>& Section::parent() const {
    const Morphology* morphology = getOwningMorphologyOrThrow();
    return morphology->_sections.at(morphology->_parent.at(id()));
}

bool Section::isRoot() const {
    const Morphology* morphology = getOwningMorphologyOrThrow();
    const auto parentId = morphology->_parent.find(id());
    if (parentId != morphology->_parent.end()) {
        return morphology->_sections.find(parentId->second) == morphology->_sections.end();
    }
    return true;
}

bool Section::hasSameShape(const Section& other) const noexcept {
    return (other.type() == type() && other.diameters() == diameters() &&
            other.points() == points() && other.perimeters() == perimeters());
}

bool Section::isHeterogeneous(bool downstream) const {
    auto predicate = [&](const std::shared_ptr<Section>& s) { return type() != s->type(); };

    if (downstream) {
        return std::any_of(breadth_begin(), breadth_end(), predicate);
    }

    return std::any_of(upstream_begin(), upstream_end(), predicate);
}

const std::vector<std::shared_ptr<Section>>& Section::children() const {
    const Morphology* morphology = getOwningMorphologyOrThrow();

    const auto it = morphology->_children.find(id());
    if (it == morphology->_children.end()) {
        static std::vector<std::shared_ptr<Section>> empty;
        return empty;
    }
    return it->second;
}

depth_iterator Section::depth_begin() const {
    throwIfNoOwningMorphology();
    return depth_iterator(const_cast<Section*>(this)->shared_from_this());
}

depth_iterator Section::depth_end() const {
    throwIfNoOwningMorphology();
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const {
    throwIfNoOwningMorphology();
    return breadth_iterator(const_cast<Section*>(this)->shared_from_this());
}

breadth_iterator Section::breadth_end() const {
    throwIfNoOwningMorphology();
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const {
    throwIfNoOwningMorphology();
    return upstream_iterator(const_cast<Section*>(this)->shared_from_this());
}

upstream_iterator Section::upstream_end() const {
    throwIfNoOwningMorphology();
    return upstream_iterator();
}

std::shared_ptr<Section> Section::appendSection(std::shared_ptr<Section> original_section,
                                                bool recursive) {
    Morphology* morphology = getOwningMorphologyOrThrow();

    const std::shared_ptr<Section> ptr(
        new Section(morphology, morphology->_counter, *original_section));
    unsigned int parentId = id();
    uint32_t childId = morphology->_register(ptr);
    auto& _sections = morphology->_sections;

    bool emptySection = _sections[childId]->points().empty();
    if (emptySection) {
        emitWarning(
            std::make_shared<AppendingEmptySection>(morphology->_uri, _sections[childId]->id()));
    }

    if (!emptySection && !_checkDuplicatePoint(_sections[parentId], _sections[childId])) {
        emitWarning(std::make_shared<WrongDuplicate>(morphology->_uri,
                                                     _sections[childId],
                                                     _sections.at(parentId)));
    }

    morphology->_parent[childId] = parentId;
    morphology->_children[parentId].push_back(ptr);

    // Careful not to use a reference here or you will face ref invalidation problem with vector
    // resize The argument `original_section` of this function could be a reference to the
    // `_children` array and that reference might be invalidated by this `push_back` (in case
    // `vector` needs to reallocate the array)
    if (recursive) {
        for (auto child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const morphio::Section& section, bool recursive) {
    Morphology* morphology = getOwningMorphologyOrThrow();
    const std::shared_ptr<Section> ptr(new Section(morphology, morphology->_counter, section));
    // const auto ptr = std::make_shared<Section>(morphology, morphology->_counter, section);
    unsigned int parentId = id();
    uint32_t childId = morphology->_register(ptr);
    auto& _sections = morphology->_sections;

    bool emptySection = _sections[childId]->points().empty();
    if (emptySection) {
        emitWarning(
            std::make_shared<AppendingEmptySection>(morphology->_uri, _sections[childId]->id()));
    }

    if (!emptySection && !_checkDuplicatePoint(_sections[parentId], _sections[childId])) {
        emitWarning(std::make_shared<WrongDuplicate>(morphology->_uri,
                                                     _sections[childId],
                                                     _sections.at(parentId)));
    }

    morphology->_parent[childId] = parentId;
    morphology->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const Property::PointLevel& pointProperties,
                                                SectionType sectionType) {
    Morphology* morphology = getOwningMorphologyOrThrow();
    unsigned int parentId = id();

    auto& _sections = morphology->_sections;
    if (sectionType == SectionType::SECTION_UNDEFINED) {
        sectionType = type();
    }

    if (sectionType == SECTION_SOMA) {
        throw morphio::SectionBuilderError("Cannot create section with type soma");
    }

    std::shared_ptr<Section> ptr(
        new Section(morphology, morphology->_counter, sectionType, pointProperties));

    uint32_t childId = morphology->_register(ptr);

    bool emptySection = _sections[childId]->points().empty();
    if (emptySection) {
        emitWarning(
            std::make_shared<AppendingEmptySection>(morphology->_uri, _sections[childId]->id()));
    }

    if (!emptySection && !_checkDuplicatePoint(_sections[parentId], _sections[childId])) {
        emitWarning(std::make_shared<WrongDuplicate>(morphology->_uri,
                                                     _sections[childId],
                                                     _sections[parentId]));
    }

    morphology->_parent[childId] = parentId;
    morphology->_children[parentId].push_back(ptr);
    return ptr;
}

void Section::emitWarning(std::shared_ptr<WarningMessage> wm) {
    getOwningMorphologyOrThrow()->getWarningHandler()->emit(std::move(wm));
}

}  // end namespace mut
}  // end namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::mut::Section& section) {
    const auto& points = section.points();
    if (points.empty()) {
        os << "Section(id=" << section.id() << ", points=[])";
    } else {
        os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., ("
           << points[points.size() - 1] << ")])";
    }

    return os;
}
