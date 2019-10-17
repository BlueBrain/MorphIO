#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>

namespace morphio {
MitoSection Mitochondria::section(uint32_t id) const {
    return {id, _properties};
}

std::vector<MitoSection> Mitochondria::sections() const {
    std::vector<MitoSection> sections_;
    for (unsigned int i = 0; i < _properties->get<morphio::Property::MitoSection>().size(); ++i) {
        sections_.push_back(section(i));
    }
    return sections_;
}

std::vector<MitoSection> Mitochondria::rootSections() const {
    std::vector<MitoSection> result;
    const auto& mitoChildren = _properties->children<morphio::Property::MitoSection>();
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
