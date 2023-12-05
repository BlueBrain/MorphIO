/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>

namespace morphio {
MitoSection Mitochondria::section(uint32_t id) const {
    return {id, properties_};
}

std::vector<MitoSection> Mitochondria::sections() const {
    std::vector<MitoSection> sections_;
    for (unsigned int i = 0; i < properties_->get<morphio::Property::MitoSection>().size(); ++i) {
        sections_.push_back(section(i));
    }
    return sections_;
}

std::vector<MitoSection> Mitochondria::rootSections() const {
    std::vector<MitoSection> result;
    const auto& mitoChildren = properties_->children<morphio::Property::MitoSection>();
    const auto& it = mitoChildren.find(-1);
    if (it != mitoChildren.end()) {
        const auto& children = it->second;

        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }
    }
    return result;
}

}  // namespace morphio
