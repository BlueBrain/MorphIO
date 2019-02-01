#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>

namespace morphio {
const MitoSection Mitochondria::section(const uint32_t& id) const
{
    return MitoSection(id, _properties);
}

const std::vector<MitoSection> Mitochondria::sections() const
{
    std::vector<MitoSection> sections;
    for (unsigned int i = 0;
         i < _properties->get<morphio::Property::MitoSection>().size(); ++i) {
        sections.push_back(section(i));
    }
    return sections;
}

const std::vector<MitoSection> Mitochondria::rootSections() const
{
    std::vector<MitoSection> result;
    try {
        const std::vector<uint32_t>& children = _properties->children<morphio::Property::MitoSection>().at(-1);

        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }

        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}

} // namespace morphio
