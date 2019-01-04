#include <morphio/mut/mito_section.h>
#include <morphio/mut/mitochondria.h>

namespace morphio {
namespace mut {
MitoSection::MitoSection(
    Mitochondria* mitochondria, int id,
    const Property::MitochondriaPointLevel& pointProperties)
    : _id(id)
    , _mitochondria(mitochondria)
    , _mitoPoints(pointProperties)
{
}

MitoSection::MitoSection(Mitochondria* mitochondria, int id,
    const morphio::MitoSection& section)
    : MitoSection(mitochondria, id,
          Property::MitochondriaPointLevel(
              section._properties->_mitochondriaPointLevel,
              section._range))
{
}

MitoSection::MitoSection(Mitochondria* mitochondria, int id,
    const MitoSection& section)
    : _id(id)
    , _mitochondria(mitochondria)
    , _mitoPoints(section._mitoPoints)
{
}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    const Property::MitochondriaPointLevel& points)
{
    int32_t parentId = id();

    std::shared_ptr<MitoSection> ptr(new MitoSection(_mitochondria,
                                         _mitochondria->_counter,
                                         points),
        friendDtorForSharedPtrMito);

    uint32_t childId = _mitochondria->_register(ptr);

    _mitochondria->_parent[childId] = parentId;
    _mitochondria->_children[parentId].push_back(ptr);
    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    std::shared_ptr<MitoSection> original_section, bool recursive)
{
    std::shared_ptr<MitoSection> ptr(new MitoSection(_mitochondria,
                                         _mitochondria->_counter,
                                         *original_section),
        friendDtorForSharedPtrMito);
    int32_t parentId = id();
    uint32_t id = _mitochondria->_register(ptr);

    _mitochondria->_parent[id] = parentId;
    _mitochondria->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> MitoSection::appendSection(
    const morphio::MitoSection& section, bool recursive)
{
    std::shared_ptr<MitoSection> ptr(new MitoSection(_mitochondria,
                                         _mitochondria->_counter,
                                         section),
        friendDtorForSharedPtrMito);
    int32_t parentId = id();
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

const std::shared_ptr<MitoSection> MitoSection::parent() const
{
    return _mitochondria->_sections.at(_mitochondria->_parent.at(id()));
}

bool MitoSection::isRoot() const
{
    try {
        parent();
        return false;
    } catch (const std::out_of_range& e) {
        return true;
    }
}

const std::vector<std::shared_ptr<MitoSection>> MitoSection::children() const
{
    try {
        return _mitochondria->_children.at(id());
    } catch (const std::out_of_range& e) {
        return std::vector<std::shared_ptr<MitoSection>>();
    }
}

} // namespace mut
} // namespace morphio
