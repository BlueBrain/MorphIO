/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <queue>  // std::queue

#include <morphio/mut/mitochondria.h>


#include "../shared_utils.hpp"

namespace morphio {
namespace mut {

Mitochondria::MitoSectionP Mitochondria::appendRootSection(const morphio::MitoSection& section_,
                                                           bool recursive) {
    auto ptr = std::make_shared<MitoSection>(this, _counter, section_);
    _register(ptr);
    root_sections_.push_back(ptr);

    if (recursive) {
        for (const auto& child : section_.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

Mitochondria::MitoSectionP Mitochondria::appendRootSection(const MitoSectionP& section_,
                                                           bool recursive) {
    auto section_copy = std::make_shared<MitoSection>(this, _counter, *section_);
    _register(section_copy);
    root_sections_.push_back(section_copy);

    if (recursive) {
        for (const auto& child : section_->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

Mitochondria::MitoSectionP Mitochondria::appendRootSection(
    const Property::MitochondriaPointLevel& pointProperties) {
    auto ptr = std::make_shared<MitoSection>(this, _counter, pointProperties);
    _register(ptr);
    root_sections_.push_back(ptr);

    return ptr;
}

static void _appendMitoProperties(Property::MitochondriaPointLevel& to,
                                  const Property::MitochondriaPointLevel& from,
                                  int offset = 0) {
    _appendVector(to._sectionIds, from._sectionIds, offset);
    _appendVector(to._relativePathLengths, from._relativePathLengths, offset);
    _appendVector(to._diameters, from._diameters, offset);
}

const std::vector<Mitochondria::MitoSectionP>& Mitochondria::children(
    const MitoSectionP& section_) const {
    const auto it = children_.find(section_->id());
    if (it == children_.end()) {
        static std::vector<Mitochondria::MitoSectionP> empty;
        return empty;
    }
    return it->second;
}

const Mitochondria::MitoSectionP& Mitochondria::parent(const MitoSectionP& parent) const {
    return section(parent_.at(parent->id()));
}

bool Mitochondria::isRoot(const MitoSectionP& section_) const {
    return parent_.count(section_->id()) == 0;
}

const Mitochondria::MitoSectionP& Mitochondria::section(uint32_t id) const {
    return sections_.at(id);
}

void Mitochondria::_buildMitochondria(Property::Properties& properties) const {
    int32_t counter = 0;
    std::map<uint32_t, int32_t> newIds;

    for (const std::shared_ptr<MitoSection>& mitoStart : root_sections_) {
        std::queue<std::shared_ptr<MitoSection>> q;
        q.push(mitoStart);
        while (!q.empty()) {
            std::shared_ptr<MitoSection> section_ = q.front();
            q.pop();
            int32_t parentOnDisk = isRoot(section_) ? -1 : newIds[parent(section_)->id()];

            properties._mitochondriaSectionLevel._sections.push_back(
                {static_cast<int>(properties._mitochondriaPointLevel._diameters.size()),
                 parentOnDisk});
            _appendMitoProperties(properties._mitochondriaPointLevel, section_->_mitoPoints);

            newIds[section_->id()] = counter++;

            for (const auto& child : children(section_)) {
                q.push(child);
            }
        }
    }
}

const std::shared_ptr<MitoSection>& Mitochondria::mitoSection(uint32_t id) const {
    return sections_.at(id);
}

mito_depth_iterator Mitochondria::depth_begin(const MitoSectionP& section) const {
    return mito_depth_iterator(section);
}

mito_depth_iterator Mitochondria::depth_end() const {
    return mito_depth_iterator();
}

mito_breadth_iterator Mitochondria::breadth_begin(const MitoSectionP& section) const {
    return mito_breadth_iterator(section);
}

mito_breadth_iterator Mitochondria::breadth_end() const {
    return mito_breadth_iterator();
}

mito_upstream_iterator Mitochondria::upstream_begin(const MitoSectionP& section) const {
    return mito_upstream_iterator(section);
}

mito_upstream_iterator Mitochondria::upstream_end() const {
    return mito_upstream_iterator();
}

uint32_t Mitochondria::_register(const MitoSectionP& section) {
    if (sections_.count(section->id()) > 0) {
        throw SectionBuilderError("Section already exists");
    }
    _counter = std::max(_counter, section->id()) + 1;

    sections_[section->id()] = section;
    return section->id();
}

}  // namespace mut
}  // namespace morphio
