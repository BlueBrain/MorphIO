#include <assert.h>

#include <sstream>
#include <string>

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/soma.h>

namespace morphio
{
namespace mut
{
Morphology::Morphology(const morphio::Morphology& morphology)
    : _counter(0)
    , _soma(std::make_shared<Soma>(morphology.soma()))
{

    for (const morphio::Section& root : morphology.rootSections())
    {
        appendSection(-1, root, true);
    }
}

uint32_t Morphology::appendSection(uint32_t parentId,
                                   const morphio::Section& section,
                                   bool recursive)
{
    uint32_t id = _register(std::shared_ptr<Section>(new Section(section),
                                                     friendDtorForSharedPtr));
    if(parentId == -1)
        _rootSections.push_back(id);
    else {
        _parent[id] = parentId;
        _children[parentId].push_back(id);
    }

    if (recursive)
    {
        for (const auto& child : section.children()){
            appendSection(id, child, true);
        }

    }

    return id;
}

uint32_t Morphology::appendSection(uint32_t parentId, SectionType type,
                                   const Property::PointLevel& pointProperties)
{
    uint32_t id = _register(std::shared_ptr<Section>(new Section(_counter, type, pointProperties),
                                                     friendDtorForSharedPtr));
    if(parentId == -1)
        _rootSections.push_back(id);
    else {
        _parent[id] = parentId;
        _children[parentId].push_back(id);
    }
    return id;
}

void friendDtorForSharedPtr(Section* section){ delete section; }

uint32_t Morphology::_register(std::shared_ptr<Section> section)
{
    _counter = std::max(_counter, section->id() + 1);
    if (_sections[section->id()])
    {
        std::stringstream ss;
        ss << "Cannot register section (" << section->id()
           << "). The morphology has already a section with the same ID."
           << std::endl;
        LBTHROW(ss.str());
    }
    _sections[section->id()] = std::shared_ptr<Section>(section);
    return section->id();
}

std::shared_ptr<Soma>& Morphology::soma()
{
    return _soma;
}

const std::vector<uint32_t>& Morphology::rootSections()
{
    return _rootSections;
}

Morphology::~Morphology()
{
    for (auto root : _rootSections)
        deleteSection(root, true);
}

const std::map<uint32_t, std::shared_ptr<Section>>& Morphology::sections() const {
    return _sections;
}

void eraseByValue(std::vector<uint32_t> &vec, uint32_t id)
{
    vec.erase(std::remove(vec.begin(), vec.end(), id), vec.end());
}

void Morphology::deleteSection(uint32_t id, bool recursive)

{
    for (auto child : _children[id])
    {
        if (recursive)
        {
            deleteSection(child, recursive);
        }
        else
        {
            // Re-link children to their "grand-parent"
            _parent[child] = _parent[id];
            _children[_parent[id]].push_back(child);
            if (_parent[id] == -1)
                _rootSections.push_back(child);
        }
    }
    eraseByValue(_rootSections, id);
    eraseByValue(_children[_parent[id]], id);
    _children.erase(id);
    _parent.erase(id);
    _sections.erase(id);
}

void Morphology::traverse(
    std::function<void(Morphology& morphology, uint32_t section)> fun,
    uint32_t startSection)
{
    auto& sections =
        startSection != -1 ? std::vector<uint32_t>{startSection} : rootSections();
    for (auto root : sections)
    {
        section(root)->traverse(*this, fun);
    }
}

template <typename T>
void _appendVector(std::vector<T>& to, const std::vector<T>& from)
{
    to.insert(to.end(), from.begin(), from.end());
}

void _appendProperties(Property::PointLevel& to,
                       const Property::PointLevel& from)
{
    _appendVector(to._points, from._points);
    _appendVector(to._diameters, from._diameters);
    _appendVector(to._perimeters, from._perimeters);
}

const Property::Properties Morphology::buildReadOnly()
{
    using std::setw;
    int i = 0;


    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties;
    auto writeSection = [&properties, &sectionIdOnDisk, &newIds]
        (Morphology& morpho, uint32_t sectionId) {
        auto section = morpho.section(sectionId);
        int parentOnDisk =
        (morpho.parent(sectionId) != -1 ? newIds[morpho.parent(sectionId)] : 1);

        int start = properties._pointLevel._points.size();
        properties._sectionLevel._sections.push_back({start, parentOnDisk});
        properties._sectionLevel._sectionTypes.push_back(section->type());
        _appendProperties(properties._pointLevel, section->_pointProperties);
        newIds[sectionId] = sectionIdOnDisk++;
    };

    _appendProperties(properties._pointLevel, soma()->_pointProperties);
    properties._sectionLevel._sections.push_back({0, -1});

    properties._sectionLevel._sectionTypes.push_back(SECTION_UNDEFINED);

    traverse(writeSection);
    return properties;
}

const uint32_t Morphology::parent(uint32_t id) {
    try {
        return _parent[id];
    } catch (const std::out_of_range &e) {
        return -1;
    }
}

const std::vector<uint32_t> Morphology::children(uint32_t id) {
    try {
        return _children[id];
    } catch (const std::out_of_range &e) {
        return std::vector<uint32_t>();
    }
}

const std::shared_ptr<Section> Morphology::section(uint32_t id){
    return _sections[id];
}

} // end namespace mut
} // end namespace morphio
