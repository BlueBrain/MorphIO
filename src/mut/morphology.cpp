#include <assert.h>

#include <sstream>
#include <string>

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/iterators.h>
#include <morphio/mut/writers.h>
#include <morphio/soma.h>


namespace morphio
{
namespace mut
{


Morphology::Morphology(const morphio::URI& uri)
    : Morphology(morphio::Morphology(uri))
{
}

Morphology::Morphology(const morphio::Morphology& morphology)
    : _counter(0)
    , _soma(std::make_shared<Soma>(morphology.soma()))
{

    for (const morphio::Section& root : morphology.rootSections())
    {
        appendSection(-1, root, true);
    }
}

bool _checkDuplicatePoint(std::shared_ptr<Section> parent,
                          std::shared_ptr<Section> current) {
    // Weird edge case where parent is empty: skipping it
    if(parent->points().empty())
        return true;

    if(current->points().empty())
        return false;

    if(parent->points()[parent->points().size()-1] != current->points()[0])
        return false;

    if(parent->diameters()[parent->diameters().size()-1] != current->diameters()[0])
        return false;

    // As perimeter is optional, it must either be defined for parent and current
    // or not be defined at all
    if(parent->perimeters().empty() != current->perimeters().empty())
        return false;

    if(!parent->perimeters().empty() &&
       parent->perimeters()[parent->perimeters().size()-1] != current->perimeters()[0])
        return false;

    return true;
}
uint32_t Morphology::appendSection(int32_t parentId,
                                   const morphio::Section& section,
                                   bool recursive)
{
    uint32_t id = _register(std::shared_ptr<Section>(new Section(section),
                                                     friendDtorForSharedPtr));
    if(parentId == -1)
        _rootSections.push_back(id);
    else {
        if(!_checkDuplicatePoint(_sections[parentId], _sections[id]))
            LBERROR(_err.WARNING_WRONG_DUPLICATE(id, parentId));

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

uint32_t Morphology::appendSection(int32_t parentId, SectionType type,
                                   const Property::PointLevel& pointProperties)
{
    uint32_t id = _register(std::shared_ptr<Section>(new Section(_counter, type, pointProperties),
                                                     friendDtorForSharedPtr));
    if(parentId == -1)
        _rootSections.push_back(id);
    else {
        if(!_checkDuplicatePoint(_sections[parentId], _sections[id]))
            LBERROR(_err.WARNING_WRONG_DUPLICATE(id, parentId));

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

std::shared_ptr<Soma> Morphology::soma()
{
    return _soma;
}

// const std::shared_ptr<Soma> Morphology::soma() const
// {
//     return _soma;
// }

const std::vector<uint32_t>& Morphology::rootSections() const
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

// void Morphology::traverse(
//     std::function<void(Morphology& morphology, uint32_t section)> fun,
//     uint32_t startSection)
// {
//     auto& sections =
//         startSection != -1 ? std::vector<uint32_t>{startSection} : rootSections();
//     for (auto root : sections)
//     {
//         section(root)->traverse(*this, fun);
//     }
// }

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

const Property::Properties Morphology::buildReadOnly() const
{
    using std::setw;
    int i = 0;
    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties;

    _appendProperties(properties._pointLevel, _soma->_pointProperties);

    // Need to fill some unused value for the soma perimeter
    // in order to get the same size for the perimeter final vector than for the point one
    bool isPerimeterUsed = _rootSections.empty() ?
        false :
        _sections.at(_rootSections.at(0))->_pointProperties._perimeters.size() > 0;

    if(isPerimeterUsed)
        properties._pointLevel._perimeters = std::vector<float>(_soma->_pointProperties._points.size(), -1);

    properties._sectionLevel._sections.push_back({0, -1});
    properties._sectionLevel._sectionTypes.push_back(SECTION_UNDEFINED);

    for(auto it = depth_begin(); it != depth_end(); ++it) {
        int32_t sectionId = *it;
        int32_t parentId = parent(sectionId);
        auto &section = _sections.at(sectionId);
        int parentOnDisk =
            (_parent.count(sectionId) > 0 ? newIds[parentId] : 0);

        if(parentId > -1 && // Exclude root sections
           !_checkDuplicatePoint(_sections.at(parentId), _sections.at(sectionId)))
            LBTHROW(SectionBuilderError(_err.WARNING_WRONG_DUPLICATE(sectionId, parentId)));


        int start = properties._pointLevel._points.size();
        properties._sectionLevel._sections.push_back({start, parentOnDisk});
        properties._sectionLevel._sectionTypes.push_back(section->type());
        _appendProperties(properties._pointLevel, section->_pointProperties);
        newIds[sectionId] = sectionIdOnDisk++;
    }

    return properties;
}

const uint32_t Morphology::parent(uint32_t id) const {
    try {
        return _parent.at(id);
    } catch (const std::out_of_range &e) {
        return -1;
    }
}

const std::vector<uint32_t> Morphology::children(uint32_t id) const {
    try {
        return _children.at(id);
    } catch (const std::out_of_range &e) {
        return std::vector<uint32_t>();
    }
}

const std::shared_ptr<Section> Morphology::section(uint32_t id) const {
    return _sections.at(id);
}

depth_iterator Morphology::depth_begin(uint32_t id) const
{
    return depth_iterator(*this, id);
}

depth_iterator Morphology::depth_end() const
{
    return depth_iterator();
}

breadth_iterator Morphology::breadth_begin(uint32_t id) const
{
    return breadth_iterator(*this, id);
}

breadth_iterator Morphology::breadth_end() const
{
    return breadth_iterator();
}

upstream_iterator Morphology::upstream_begin(uint32_t id) const
{
    return upstream_iterator(*this, id);
}

upstream_iterator Morphology::upstream_end() const
{
    return upstream_iterator();
}


void Morphology::write_asc(const std::string& filename) {
    // writer::asc(filename);
}

void Morphology::write_swc(const std::string& filename) {
    // writer::swc(filename);
}

void Morphology::write_h5(const std::string& filename) {
    // writer::h5(filename);
}


} // end namespace mut
} // end namespace morphio
