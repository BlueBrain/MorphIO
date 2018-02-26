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
        uint32_t id = _register(std::shared_ptr<Section>(new Section(root),
                                                          friendDtorForSharedPtr));
        _rootSections.insert(id);
    }
}

uint32_t Morphology::appendSection(uint32_t parentId,
                                   const morphio::Section& section,
                                   bool recursive)
{
    uint32_t id = _register(std::shared_ptr<Section>(new Section(section),
                                                     friendDtorForSharedPtr));
    if(parentId == -1)
        _rootSections.insert(id);
    else {
        _parent[id] = parentId;
        _children[parentId].insert(id);
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
        _rootSections.insert(id);
    else {
        _parent[id] = parentId;
        _children[parentId].insert(id);
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

std::shared_ptr<Soma> Morphology::soma()
{
    return _soma;
}

const std::set<uint32_t>& Morphology::rootSections()
{
    return _rootSections;
}

Morphology::~Morphology()
{
    for (auto root : _rootSections)
        deleteSection(root, true);
}

std::map<uint32_t, std::shared_ptr<Section>>& Morphology::sections() {
    return _sections;
}

void Morphology::deleteSection(uint32_t id, bool recursive)

{
    _rootSections.erase(id);
    for (auto child : _children[id])
    {
        if (recursive)
            deleteSection(child, recursive);
        else
        {
            // Re-link children to their "grand-parent"
            _parent[child] = _parent[id];
            _children[_parent[id]].insert(child);
            if (_parent[id] == -1)
                _rootSections.insert(child);
        }
    }
    _children.erase(id);
    _parent.erase(id);
    _sections.erase(id);
}

void Morphology::traverse(
    std::function<void(Morphology& morphology, uint32_t section)> fun,
    uint32_t startSection)
{
    auto& sections =
        startSection ? std::set<uint32_t>{startSection} : rootSections();
    for (auto root : sections)
    {
        _sections[root]->traverse(*this, fun);
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

Property::Properties Morphology::buildReadOnly()
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

namespace writer
{
void swc(Morphology& morphology)
{
    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    auto writeSection =
        [&sectionIdOnDisk, &newIds](Morphology& morphology, uint32_t sectionId) {
        auto section = morphology.section(sectionId);
            const auto& points = section->points();
            const auto& diameters = section->diameters();

            assert(points.size() > 0 && "Empty section");
            for (int i = 0; i < points.size(); ++i)
            {
                std::cout << sectionIdOnDisk << ' ' << section->type() << ' '
                          << points[i][0] << ' ' << points[i][1] << ' '
                          << points[i][2] << ' ' << diameters[i] / 2. << ' ';
                if (i > 0)
                    std::cout << sectionIdOnDisk - 1 << std::endl;
                else {
                    uint32_t parentId = morphology.parent(sectionId);
                    std::cout << (parentId == -1 ? newIds[parentId] : 1) << std::endl;
                }

                ++sectionIdOnDisk;
            }
            newIds[section->id()] = sectionIdOnDisk - 1;
        };

    morphology.traverse(writeSection);
}

void _write_asc_points(const Points& points,
                       const std::vector<float>& diameters, int indentLevel)
{
    for (int i = 0; i < points.size(); ++i)
    {
        std::cout << std::string(indentLevel, ' ') << "(" << points[i][0] << ' '
                  << points[i][1] << ' ' << points[i][2] << ' ' << diameters[i]
                  << ')' << std::endl;
    }
}

void _write_asc_section(Morphology& morpho, uint32_t id, int indentLevel)
{
    std::string indent(indentLevel, ' ');
    auto section = morpho.section(id);
    _write_asc_points(section->points(), section->diameters(), indentLevel);

    if (!morpho.children(id).empty())
    {
        std::cout << indent << "(" << std::endl;
        auto it = morpho.children(id).begin();
        _write_asc_section(morpho, (*it++), indentLevel + 2);
        for (; it != morpho.children(id).end(); ++it)
        {
            std::cout << indent << "|" << std::endl;
            _write_asc_section(morpho, (*it), indentLevel + 2);
        }
        std::cout << indent << ")" << std::endl;
    }
}
void asc(Morphology& morphology)
{
    std::map<morphio::SectionType, std::string> header;
    header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";

    const auto soma = morphology.soma();
    std::cout << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
    _write_asc_points(soma->points(), soma->diameters(), 2);
    std::cout << ")\n\n";

    for (auto& id : morphology.rootSections())
    {
        std::cout << header[morphology.section(id)->type()];
        _write_asc_section(morphology, id, 2);
        std::cout << ")\n\n";
    }
}

void h5(Morphology& morphology)
{
    using std::setw;
    int i = 0;

    int start = 0;
    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties;
    auto writeSection = [&start, &properties, &sectionIdOnDisk,
                         &newIds](Morphology& morpho, uint32_t sectionId) {
        uint32_t parentId = morpho.parent(sectionId);
        int parentOnDisk = (parentId != -1 ? newIds[parentId] : 1);
        auto section = morpho.section(sectionId);
        std::cout << setw(6) << sectionIdOnDisk << ' ' << setw(6)
                  << parentOnDisk << ' ' << setw(6) << start << ' ' << setw(6)
                  << section->type() << std::endl;
        newIds[section->id()] = sectionIdOnDisk++;
        start += section->points().size();
    };

    // Hard-coding soma line. Not so swag
    // Will we have morphology without soma ?
    std::cout << setw(6) << 0 << ' ' << setw(6) << -1 << ' ' << setw(6) << start
              << ' ' << setw(6) << SECTION_SOMA << std::endl;

    start += morphology.soma()->points().size();
    morphology.traverse(writeSection);
}

} // end namespace writer
} // end namespace mut
} // end namespace morphio
