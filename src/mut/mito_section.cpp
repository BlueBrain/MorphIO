#include <morphio/mut/mito_section.h>
#include <morphio/mut/mitochondria.h>

namespace morphio {
namespace mut {
MitoSection::MitoSection(Mitochondria* mitochondria,
                         unsigned int id_,
                         const Property::MitochondriaPointLevel& pointProperties)
    : _id(id_)
    , _mitochondria(mitochondria)
    , _mitoPoints(pointProperties) {}

MitoSection::MitoSection(Mitochondria* mitochondria,
                         unsigned int id_,
                         const morphio::MitoSection& section)
    : MitoSection(mitochondria,
                  id_,
                  Property::MitochondriaPointLevel(section._properties->_mitochondriaPointLevel,
                                                   section._range)) {}

MitoSection::MitoSection(Mitochondria* mitochondria, unsigned int id_, const MitoSection& section)
    : _id(id_)
    , _mitochondria(mitochondria)
    , _mitoPoints(section._mitoPoints) {}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    const Property::MitochondriaPointLevel& points) {
    unsigned int parentId = id();

    std::shared_ptr<MitoSection> ptr(
        new MitoSection(_mitochondria, _mitochondria->_counter, points));

    uint32_t childId = _mitochondria->_register(ptr);

    _mitochondria->_parent[childId] = parentId;
    _mitochondria->_children[parentId].push_back(ptr);
    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    const std::shared_ptr<MitoSection>& original_section, bool recursive) {
    std::shared_ptr<MitoSection> ptr(
        new MitoSection(_mitochondria, _mitochondria->_counter, *original_section));
    unsigned int parentId = id();
    uint32_t id_ = _mitochondria->_register(ptr);

    _mitochondria->_parent[id_] = parentId;
    _mitochondria->_children[parentId].push_back(ptr);

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
        new MitoSection(_mitochondria, _mitochondria->_counter, section));
    unsigned int parentId = id();
    uint32_t childId = _mitochondria->_register(ptr);

    _mitochondria->_parent[childId] = parentId;
    _mitochondria->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::parent() const {
    return _mitochondria->_sections.at(_mitochondria->_parent.at(id()));
}

bool MitoSection::isRoot() const {
    try {
        parent();
        return false;
    } catch (const std::out_of_range&) {
        return true;
    }
}

const std::vector<std::shared_ptr<MitoSection>>& MitoSection::children() const {
    const auto& children = _mitochondria->_children;
    const auto it = children.find(id());
    if (it == children.end()) {
        static std::vector<std::shared_ptr<MitoSection>> empty;
        return empty;
    }
    return it->second;
}

}  // namespace mut
}  // namespace morphio
