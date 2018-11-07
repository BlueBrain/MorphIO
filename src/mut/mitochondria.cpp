#include <morphio/mut/mitochondria.h>
#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/writers.h>

namespace morphio
{
namespace mut
{
std::shared_ptr<MitoSection> Mitochondria::appendSection(
    std::shared_ptr<MitoSection> parent, const morphio::MitoSection section,
    bool recursive)
{
    uint32_t mitoId = _mitochondriaSectionCounter++;
    std::shared_ptr<MitoSection> ptr(new MitoSection(mitoId, section));
    _sections[mitoId] = ptr;

    int32_t mitoParentId = parent ? parent->id() : -1;


    if(parent == nullptr)
        _rootSections.push_back(ptr);
    else
        _parent[mitoId] = mitoParentId;


    try
    {
        _children[mitoParentId].push_back(ptr);
    }
    catch (const std::out_of_range& oor)
    {
        LBTHROW(morphio::plugin::ErrorMessages().ERROR_MISSING_MITO_PARENT(mitoParentId));
    }

    if (recursive)
    {
        for (const auto child : section.children()){
            appendSection(ptr, child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> Mitochondria::appendSection(
    std::shared_ptr<MitoSection> parent, const Property::MitochondriaPointLevel& points)
{
    uint32_t mitoId = _mitochondriaSectionCounter;
    int32_t mitoParentId = parent ? parent->id() : -1;

    if(mitoParentId > -1)
        _parent[mitoId] = mitoParentId;

    std::shared_ptr<MitoSection> ptr(new MitoSection(mitoId, points));
    _sections[mitoId] = ptr;

    if (mitoParentId == -1)
        _rootSections.push_back(ptr);

    try
    {
        _children[mitoParentId].push_back(ptr);
    }
    catch (const std::out_of_range& oor)
    {
        LBTHROW(morphio::plugin::ErrorMessages().ERROR_MISSING_MITO_PARENT(mitoParentId));
    }

    ++_mitochondriaSectionCounter;
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

const std::vector<std::shared_ptr<MitoSection>> Mitochondria::children(std::shared_ptr<MitoSection> section) const
{
    try
    {
        return _children.at(section->id());
    }
    catch (const std::out_of_range& e)
    {
        return std::vector<std::shared_ptr<MitoSection>>();
    }
}

const std::vector<std::shared_ptr<MitoSection>>& Mitochondria::rootSections() const
{
    return _rootSections;
}

const std::shared_ptr<MitoSection> Mitochondria::parent(const std::shared_ptr<MitoSection> parent) const {
    return section(_parent.at(parent->id()));
}

bool Mitochondria::isRoot(const std::shared_ptr<MitoSection> section) const {
    try {
        parent(section);
        return false;
    } catch (const std::out_of_range &e) {
        return true;
    }
}

const std::shared_ptr<MitoSection> Mitochondria::section(uint32_t id) const
{
    return _sections.at(id);
}

const std::map<uint32_t, std::shared_ptr<MitoSection>> Mitochondria::sections() const {
    return _sections;
}


void Mitochondria::_buildMitochondria(Property::Properties& properties) const
{
    uint32_t counter = 0;
    std::map<uint32_t, uint32_t> newIds;

    for (std::shared_ptr<MitoSection> mitoStart : _rootSections)
    {
        std::queue<std::shared_ptr<MitoSection>> q;
        q.push(mitoStart);
        while (!q.empty())
        {
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



mito_depth_iterator Mitochondria::depth_begin(std::shared_ptr<MitoSection> section) const
{
    return mito_depth_iterator(*this, section);
}

mito_depth_iterator Mitochondria::depth_end() const
{
    return mito_depth_iterator();
}

mito_breadth_iterator Mitochondria::breadth_begin(std::shared_ptr<MitoSection> section) const
{
    return mito_breadth_iterator(*this, section);
}

mito_breadth_iterator Mitochondria::breadth_end() const
{
    return mito_breadth_iterator();
}

mito_upstream_iterator Mitochondria::upstream_begin(std::shared_ptr<MitoSection> section) const
{
    return mito_upstream_iterator(*this, section);
}

mito_upstream_iterator Mitochondria::upstream_end() const
{
    return mito_upstream_iterator();
}

}
}
