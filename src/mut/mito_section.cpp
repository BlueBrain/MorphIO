/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/mut/mito_section.h>
#include <morphio/mut/mitochondria.h>

namespace morphio {
namespace mut {
MitoSection::MitoSection(Mitochondria* mitochondria,
                         unsigned int section_id,
                         const Property::MitochondriaPointLevel& pointProperties)
    : id_(section_id)
    , mitochondria_(mitochondria)
    , _mitoPoints(pointProperties) {}

MitoSection::MitoSection(Mitochondria* mitochondria,
                         unsigned int section_id,
                         const morphio::MitoSection& section)
    : MitoSection(mitochondria,
                  section_id,
                  Property::MitochondriaPointLevel(section.properties_->_mitochondriaPointLevel,
                                                   section.range_)) {}

MitoSection::MitoSection(Mitochondria* mitochondria,
                         unsigned int section_id,
                         const MitoSection& section)
    : id_(section_id)
    , mitochondria_(mitochondria)
    , _mitoPoints(section._mitoPoints) {}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    const Property::MitochondriaPointLevel& points) {
    unsigned int parentId = id();

    std::shared_ptr<MitoSection> ptr(
        new MitoSection(mitochondria_, mitochondria_->_counter, points));

    uint32_t childId = mitochondria_->_register(ptr);

    mitochondria_->parent_[childId] = parentId;
    mitochondria_->children_[parentId].push_back(ptr);
    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    const std::shared_ptr<MitoSection>& original_section, bool recursive) {
    std::shared_ptr<MitoSection> ptr(
        new MitoSection(mitochondria_, mitochondria_->_counter, *original_section));
    unsigned int parentId = id();
    uint32_t section_id = mitochondria_->_register(ptr);

    mitochondria_->parent_[section_id] = parentId;
    mitochondria_->children_[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::appendSection(const morphio::MitoSection& section,
                                                        bool recursive) {
    std::shared_ptr<MitoSection> ptr(
        new MitoSection(mitochondria_, mitochondria_->_counter, section));
    unsigned int parentId = id();
    uint32_t childId = mitochondria_->_register(ptr);

    mitochondria_->parent_[childId] = parentId;
    mitochondria_->children_[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::parent() const {
    return mitochondria_->sections_.at(mitochondria_->parent_.at(id()));
}

bool MitoSection::isRoot() const {
    return mitochondria_->parent_.count(id()) == 0;
}

bool MitoSection::hasSameShape(const MitoSection& other) const noexcept {
    return (other.neuriteSectionIds() == neuriteSectionIds() && other.diameters() == diameters() &&
            other.pathLengths() == pathLengths());
}

const std::vector<std::shared_ptr<MitoSection>>& MitoSection::children() const {
    const auto& children = mitochondria_->children_;
    const auto it = children.find(id());
    if (it == children.end()) {
        static std::vector<std::shared_ptr<MitoSection>> empty;
        return empty;
    }
    return it->second;
}

}  // namespace mut
}  // namespace morphio
