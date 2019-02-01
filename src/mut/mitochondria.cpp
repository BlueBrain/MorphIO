#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/writers.h>

namespace morphio {
namespace mut {
void friendDtorForSharedPtrMito(MitoSection* section)
{
    delete section;
}

std::shared_ptr<MitoSection> Mitochondria::appendRootSection(
    const morphio::MitoSection& section, bool recursive)
{
    std::shared_ptr<MitoSection> ptr(new MitoSection(this, _counter, section),
        friendDtorForSharedPtrMito);
    _register(ptr);
    _rootSections.push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> Mitochondria::appendRootSection(
    std::shared_ptr<MitoSection> section, bool recursive)
{
    std::shared_ptr<MitoSection> section_copy(new MitoSection(this, _counter,
                                                  *section),
        friendDtorForSharedPtrMito);
    _register(section_copy);
    _rootSections.push_back(section_copy);

    if (recursive) {
        for (const auto child : section->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

std::shared_ptr<MitoSection> Mitochondria::appendRootSection(
    const Property::MitochondriaPointLevel& pointProperties)
{
    std::shared_ptr<MitoSection> ptr(new MitoSection(this, _counter,
                                         pointProperties),
        friendDtorForSharedPtrMito);
    _register(ptr);
    _rootSections.push_back(ptr);

    return ptr;
}

void _appendMitoProperties(Property::MitochondriaPointLevel& to,
    const Property::MitochondriaPointLevel& from,
    int offset = 0)
{
    Property::_appendVector(to._sectionIds, from._sectionIds, offset);
    Property::_appendVector(to._relativePathLengths, from._relativePathLengths,
        offset);
    Property::_appendVector(to._diameters, from._diameters, offset);
}

const std::vector<std::shared_ptr<MitoSection>> Mitochondria::children(
    std::shared_ptr<MitoSection> section) const
{
    try {
        return _children.at(section->id());
    } catch (const std::out_of_range& e) {
        return std::vector<std::shared_ptr<MitoSection>>();
    }
}

const std::vector<std::shared_ptr<MitoSection>>& Mitochondria::rootSections()
    const
{
    return _rootSections;
}

const std::shared_ptr<MitoSection> Mitochondria::parent(
    const std::shared_ptr<MitoSection> parent) const
{
    return section(_parent.at(parent->id()));
}

bool Mitochondria::isRoot(const std::shared_ptr<MitoSection> section) const
{
    try {
        parent(section);
        return false;
    } catch (const std::out_of_range& e) {
        return true;
    }
}

const std::shared_ptr<MitoSection> Mitochondria::section(uint32_t id) const
{
    return _sections.at(id);
}

const std::map<uint32_t, std::shared_ptr<MitoSection>> Mitochondria::sections()
    const
{
    return _sections;
}

void Mitochondria::_buildMitochondria(Property::Properties& properties) const
{
    uint32_t counter = 0;
    std::map<uint32_t, uint32_t> newIds;

    for (std::shared_ptr<MitoSection> mitoStart : _rootSections) {
        std::queue<std::shared_ptr<MitoSection>> q;
        q.push(mitoStart);
        while (!q.empty()) {
            std::shared_ptr<MitoSection> section = q.front();
            q.pop();
            bool root = isRoot(section);
            int32_t parentOnDisk = root ? -1 : newIds[parent(section)->id()];

            properties._mitochondriaSectionLevel._sections.push_back(
                {(int)properties._mitochondriaPointLevel._diameters.size(),
                    parentOnDisk});
            _appendMitoProperties(properties._mitochondriaPointLevel,
                section->_mitoPoints);

            newIds[section->id()] = counter++;

            for (auto child : children(section))
                q.push(child);
        }
    }
}

const std::shared_ptr<MitoSection> Mitochondria::mitoSection(uint32_t id) const
{
    return _sections.at(id);
}

mito_depth_iterator Mitochondria::depth_begin(
    std::shared_ptr<MitoSection> section) const
{
    return mito_depth_iterator(*this, section);
}

mito_depth_iterator Mitochondria::depth_end() const
{
    return mito_depth_iterator();
}

mito_breadth_iterator Mitochondria::breadth_begin(
    std::shared_ptr<MitoSection> section) const
{
    return mito_breadth_iterator(*this, section);
}

mito_breadth_iterator Mitochondria::breadth_end() const
{
    return mito_breadth_iterator();
}

mito_upstream_iterator Mitochondria::upstream_begin(
    std::shared_ptr<MitoSection> section) const
{
    return mito_upstream_iterator(*this, section);
}

mito_upstream_iterator Mitochondria::upstream_end() const
{
    return mito_upstream_iterator();
}

uint32_t Mitochondria::_register(std::shared_ptr<MitoSection> section)
{
    if (_sections.count(section->id()))
        LBTHROW(SectionBuilderError("Section already exists"));
    _counter = std::max(_counter, section->id()) + 1;

    _sections[section->id()] = section;
    return section->id();
}

} // namespace mut
} // namespace morphio
