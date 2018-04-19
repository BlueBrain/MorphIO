#include <morphio/mut/mitochondria.h>
#include <morphio/mut/writers.h>

namespace morphio
{
namespace mut
{
uint32_t Mitochondria::appendSection(
    int32_t mitoParentId, const Property::MitochondriaPointLevel& points)
{
    uint32_t mitoId = ++_mitochondriaSectionCounter;

    _parent[mitoId] = mitoParentId;

    if (mitoParentId == -1)
        _rootSections.push_back(mitoId);

    try
    {
        _children[mitoParentId].push_back(mitoId);
    }
    catch (const std::out_of_range& oor)
    {
        LBTHROW("Missing mitochondria parent section ID: " +
                std::to_string(mitoParentId));
    }

    _sections[mitoId] = std::shared_ptr<MitoSection>(new MitoSection(points));
    return mitoId;
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

const std::vector<uint32_t> Mitochondria::children(uint32_t id) const
{
    try
    {
        return _children.at(id);
    }
    catch (const std::out_of_range& e)
    {
        return std::vector<uint32_t>();
    }
}

const std::vector<uint32_t>& Mitochondria::rootSections() const
{
    return _rootSections;
}

const int32_t Mitochondria::parent(uint32_t id) const {
    try {
        return _parent.at(id);
    } catch (const std::out_of_range &e) {
        return -1;
    }
}

const bool Mitochondria::isRoot(uint32_t id) const {
    return parent(id) == -1;
}


const std::shared_ptr<MitoSection> Mitochondria::section(uint32_t id) const
{
    return _sections.at(id);
}

const std::map<uint32_t, std::shared_ptr<MitoSection>>& Mitochondria::sections() const {
    return _sections;
}


void Mitochondria::_buildMitochondria(Property::Properties& properties) const
{
    uint32_t counter = 0;
    uint32_t sectionIdOnDisk = 0;
    std::map<uint32_t, uint32_t> newIds;

    for (uint32_t mitoStart : _rootSections)
    {
        std::queue<uint32_t> q;
        q.push(mitoStart);
        while (!q.empty())
        {
            uint32_t id = q.front();
            q.pop();
            int32_t parentId = _parent.at(id);
            int32_t parentOnDisk = parentId > -1 ? newIds[parentId] : -1;
            properties._mitochondriaSectionLevel._sections.push_back(
                {(int)properties._mitochondriaPointLevel._diameters.size(),
                 parentOnDisk});
            _appendMitoProperties(properties._mitochondriaPointLevel,
                                  mitoSection(id)->_mitoPoints);

            newIds[id] = counter++;

            if(_children.count(id) > 0){
                for (auto& childId : _children.at(id))
                    q.push(childId);
            }
        }
    }
}

const std::shared_ptr<MitoSection> Mitochondria::mitoSection(uint32_t id) const
{
    return _sections.at(id);
}
}
}
