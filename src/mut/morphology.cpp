#include <assert.h>

#include <sstream>
#include <string>

#include <morphio/mitochondria.h>
#include <morphio/mito_section.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/iterators.h>
#include <morphio/mut/writers.h>
#include <morphio/soma.h>


namespace morphio
{
namespace mut
{

using morphio::plugin::ErrorMessages;
Morphology::Morphology(const morphio::URI& uri, unsigned int options)
    : Morphology(morphio::Morphology(uri, options))
{
}

Morphology::Morphology(const morphio::Morphology& morphology)
    : _counter(0)
    , _soma(std::make_shared<Soma>(morphology.soma()))
{

    _cellProperties = std::make_shared<morphio::Property::CellLevel>
        (morphology._properties->_cellLevel);


    for (const morphio::Section& root : morphology.rootSections())
    {
        appendSection(nullptr, root, true);
    }

    for (const morphio::MitoSection& root : morphology.mitochondria().rootSections())
    {
        mitochondria().appendSection(-1, root, true);
    }
}

/**
   Return false if there is no duplicate point
 **/
bool _checkDuplicatePoint(std::shared_ptr<Section> parent,
                          std::shared_ptr<Section> current) {
    // Weird edge case where parent is empty: skipping it
    if(parent->points().empty())
        return true;

    if(current->points().empty())
        return false;

    if(parent->points()[parent->points().size()-1] != current->points()[0])
        return false;

    // TODO: I dont know if it is OK for the diameter and the perimeter to not
    // be duplicated

    // if(parent->diameters()[parent->diameters().size()-1] != current->diameters()[0])
    //     return false;

    // // As perimeter is optional, it must either be defined for parent and current
    // // or not be defined at all
    // if(parent->perimeters().empty() != current->perimeters().empty())
    //     return false;

    // if(!parent->perimeters().empty() &&
    //    parent->perimeters()[parent->perimeters().size()-1] != current->perimeters()[0])
    //     return false;

    return true;
}

std::shared_ptr<Section> Morphology::appendSection(std::shared_ptr<Section> parent,
                                   const morphio::Section& section,
                                   bool recursive)
{
    std::shared_ptr<Section> ptr(new Section(this, _counter, section), friendDtorForSharedPtr);
    int32_t parentId = parent == nullptr ? -1 : parent->id();

    uint32_t id = _register(ptr);
    if(parentId == -1)
        _rootSections.push_back(ptr);
    else {
      if(!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) &&
         !_checkDuplicatePoint(_sections[parentId], _sections[id]))
        LBERROR(Warning::WRONG_DUPLICATE, _err.WARNING_WRONG_DUPLICATE(_sections[id], _sections.at(parentId)));

        _parent[id] = parentId;
        _children[parentId].push_back(ptr);

    }

    if (recursive)
    {
        for (const auto& child : section.children()){
            appendSection(ptr, child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Morphology::appendSection(std::shared_ptr<Section> parent,
                                                   std::shared_ptr<Section> section,
                                   const Morphology& morphology)
{
    std::shared_ptr<Section> original_section = section;
    uint32_t id = _register(section);
    int32_t parentId = parent == nullptr ? -1 : parent->id();
    if(parentId == -1)
        _rootSections.push_back(section);
    else {
      if(!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) &&
         !_checkDuplicatePoint(_sections[parentId], _sections[id]))
        LBERROR(Warning::WRONG_DUPLICATE, _err.WARNING_WRONG_DUPLICATE(_sections[id], _sections.at(parentId)));

        _parent[id] = parentId;
        _children[parentId].push_back(section);

    }

    for (const auto& child : original_section->children()){
        appendSection(section, child, morphology);
    }

    return section;
}

std::shared_ptr<Section> Morphology::appendSection(std::shared_ptr<Section> parent,
                                                   const Property::PointLevel& pointProperties,
                                                   SectionType type)
{
    int32_t parentId = parent == nullptr ? -1 : parent->id();

    if(type == SectionType::SECTION_UNDEFINED) {
        if(parentId == -1)
            LBTHROW(SectionBuilderError(_err.ERROR_UNSPECIFIED_SECTION_TYPE()));
        else
            type = _sections[parentId] -> type();
    }

    std::shared_ptr<Section> ptr(new Section(this,
                                             _counter, type, pointProperties), friendDtorForSharedPtr);
    uint32_t id = _register(ptr);



        if(parentId == -1) {
        _rootSections.push_back(ptr);
    } else {
          if(!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) &&
             !_checkDuplicatePoint(_sections[parentId], _sections[id]))
            LBERROR(Warning::WRONG_DUPLICATE, _err.WARNING_WRONG_DUPLICATE(_sections[id], _sections[parentId]));

        _parent[id] = parentId;
        _children[parentId].push_back(ptr);
    }
    return ptr;
}

void friendDtorForSharedPtr(Section* section){ delete section; }

uint32_t Morphology::_register(std::shared_ptr<Section>& section)
{
    // If a section with the same ID already exists, we copy the node under a new ID
    if (_sections.count(section->id()))
    {
        section = std::shared_ptr<Section>(new Section(this, _counter, *section),
                                           friendDtorForSharedPtr);

    }
    _counter = std::max(_counter, section->id()) + 1;

    _sections[section->id()] = std::shared_ptr<Section>(section);
    return section->id();
}

std::shared_ptr<Soma> Morphology::soma()
{
    return _soma;
}

const std::shared_ptr<Soma> Morphology::soma() const
{
    return _soma;
}

const std::vector<std::shared_ptr<Section>>& Morphology::rootSections() const
{
    return _rootSections;
}

Morphology::~Morphology()
{
    auto roots = _rootSections; // Need to iterate on a copy
    for (auto root : roots) {
        deleteSection(root, true);
    }

}

const std::map<uint32_t, std::shared_ptr<Section>> Morphology::sections() const {
    return _sections;
}

void eraseByValue(std::vector<std::shared_ptr<Section>> &vec, std::shared_ptr<Section> section)
{
    vec.erase(std::remove(vec.begin(), vec.end(), section), vec.end());
}

void Morphology::deleteSection(std::shared_ptr<Section> section, bool recursive)

{
    if(!section)
        return;
    int id = section->id();

    if (recursive) {
        // The deletion must start by the furthest leaves, otherwise you may cut the
        // topology and forget to remove some sections
        std::vector<std::shared_ptr<Section>> ids;
        for(auto it = breadth_begin(section); it != breadth_end(); ++it) {
            ids.push_back(*it);
        }

        for(auto it = ids.rbegin(); it != ids.rend(); ++it) {
            auto child = *it;
            {
                deleteSection(child, false);
            }
        }
    } else {
        for (auto child : section->children()) {
        // Re-link children to their "grand-parent"
            _parent[id] = _parent[id];
            _children[_parent[id]].push_back(child);
            if (_parent[id] == -1)
                _rootSections.push_back(child);
        }

        eraseByValue(_rootSections, section);
        eraseByValue(_children[_parent[id]], section);
        _children.erase(id);
        _parent.erase(id);
        _sections.erase(id);
    }

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


void _appendProperties(Property::PointLevel& to,
                       const Property::PointLevel& from,
                       int offset = 0)
{
    Property::_appendVector(to._points, from._points, offset);
    Property::_appendVector(to._diameters, from._diameters, offset);

    if(!from._perimeters.empty())
        Property::_appendVector(to._perimeters, from._perimeters, offset);
}

const Property::Properties Morphology::buildReadOnly() const {
    return buildReadOnly(morphio::plugin::DebugInfo());
}

const Property::Properties Morphology::buildReadOnly(const morphio::plugin::DebugInfo& debugInfo) const
{
    using std::setw;
    int i = 0;
    int sectionIdOnDisk = 0;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties;
    morphio::plugin::ErrorMessages err(debugInfo._filename);

    if(_cellProperties) {
        properties._cellLevel = *_cellProperties;
        properties._cellLevel._somaType = _soma->type();
    }
    _appendProperties(properties._somaLevel, _soma->_pointProperties);

    for(auto it = depth_begin(); it != depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        int parentId = section->isRoot() ? -1 : section->parent()->id();
        int sectionId = section->id();
        int parentOnDisk = (section->isRoot() ? -1 : newIds[parentId]);

        if(!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) &&
           !section->isRoot() &&
           !_checkDuplicatePoint(section->parent(), section))
          LBERROR(Warning::WRONG_DUPLICATE, err.WARNING_WRONG_DUPLICATE(section, section->parent()));

        bool isUnifurcation = !section->isRoot() && section->parent()->children().size() == 1;

        // This "if" condition ensures that "unifurcations" (ie. successive sections with only 1
        // child) get merged together into a bigger section
        if(!isUnifurcation) {
            int start = properties._pointLevel._points.size();
            properties._sectionLevel._sections.push_back({start, parentOnDisk});
            properties._sectionLevel._sectionTypes.push_back(section->type());
            newIds[sectionId] = sectionIdOnDisk++;
            _appendProperties(properties._pointLevel, section->_pointProperties);
        } else {
            std::string errorMsg = err.WARNING_ONLY_CHILD(debugInfo, parentId, sectionId);
            LBERROR(Warning::ONLY_CHILD, errorMsg);

            properties._annotations.push_back(Property::Annotation(SINGLE_CHILD,
                                                                   parentId,
                                                                   section->_pointProperties,
                                                                   errorMsg,
                                                                   debugInfo.getLineNumber(sectionId)));
            newIds[sectionId] = newIds[parentId];

            // Skip the duplicate point
            _appendProperties(properties._pointLevel, section->_pointProperties, 1);
        }
    }

    mitochondria()._buildMitochondria(properties);
    return properties;
}
const std::shared_ptr<Section> Morphology::section(uint32_t id) const {
    return _sections.at(id);
}

depth_iterator Morphology::depth_begin() const
{
    return depth_iterator(*this);
}
depth_iterator Morphology::depth_begin(const std::shared_ptr<Section>& section) const
{
    return depth_iterator(*this, section);
}

depth_iterator Morphology::depth_end() const
{
    return depth_iterator();
}

breadth_iterator Morphology::breadth_begin() const
{
    return breadth_iterator(*this);
}

breadth_iterator Morphology::breadth_begin(const std::shared_ptr<Section>& section) const
{
    return breadth_iterator(*this, section);
}

breadth_iterator Morphology::breadth_end() const
{
    return breadth_iterator();
}

upstream_iterator Morphology::upstream_begin(const std::shared_ptr<Section>& section) const
{
    return upstream_iterator(*this, section);
}

upstream_iterator Morphology::upstream_end() const
{
    return upstream_iterator();
}


void Morphology::applyModifiers(unsigned int modifierFlags) {
    if(modifierFlags & NO_DUPLICATES & TWO_POINTS_SECTIONS)
        LBTHROW(SectionBuilderError(
                    _err.ERROR_UNCOMPATIBLE_FLAGS(NO_DUPLICATES,
                                                  TWO_POINTS_SECTIONS)));

    if(modifierFlags & SOMA_SPHERE)
        modifiers::soma_sphere(*this);

    if(modifierFlags & NO_DUPLICATES)
        modifiers::no_duplicate_point(*this);

    if(modifierFlags & TWO_POINTS_SECTIONS)
        modifiers::two_points_sections(*this);

    if(modifierFlags & NRN_ORDER)
        modifiers::nrn_order(*this);

}

void Morphology::write(const std::string& filename) {
    const size_t pos = filename.find_last_of(".");
    assert(pos != std::string::npos);

    std::string extension;

    // going through morphio::Morphology allows us to benefit
    // from its curation process:
    // - check for duplicate
    // - section with only one child
    morphio::mut::Morphology clean(morphio::Morphology(*this));


    for(auto& c : filename.substr(pos))
        extension += std::tolower(c);

    if (extension == ".h5")
        writer::h5(clean, filename);
    else if (extension == ".asc")
        writer::asc(clean, filename);
    else if (extension == ".swc")
        writer::swc(clean, filename);
    else
        LBTHROW(UnknownFileType(_err.ERROR_WRONG_EXTENSION(filename)));

}

void Morphology::_write_asc(const std::string& filename) {
    writer::asc(*this, filename);
}

void Morphology::_write_swc(const std::string& filename) {
    writer::swc(*this, filename);
}

void Morphology::_write_h5(const std::string& filename) {
    for(auto it = depth_begin(); it != depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;

        if(!section->isRoot() &&
           !_checkDuplicatePoint(section->parent(), section))
            LBTHROW(SectionBuilderError(_err.WARNING_WRONG_DUPLICATE(section,
                                                                     section->parent())));
    }

    writer::h5(*this, filename);
}


} // end namespace mut
} // end namespace morphio
