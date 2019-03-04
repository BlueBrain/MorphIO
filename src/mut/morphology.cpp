#include <assert.h>

#include <sstream>
#include <string>

#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/mut/iterators.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/soma.h>

namespace morphio {
namespace mut {
using morphio::plugin::ErrorMessages;
Morphology::Morphology(const morphio::URI& uri, unsigned int options)
    : Morphology(morphio::Morphology(uri, options))
{
}

Morphology::Morphology(const morphio::mut::Morphology& morphology, unsigned int options)
    : _counter(0)
    , _soma(std::make_shared<Soma>(*morphology.soma()))
{
    _cellProperties = std::make_shared<morphio::Property::CellLevel>(
        *morphology._cellProperties);

    for (const std::shared_ptr<Section> root : morphology.rootSections()) {
        appendRootSection(root, true);
    }

    for (const std::shared_ptr<MitoSection> root :
        morphology.mitochondria().rootSections()) {
        mitochondria().appendRootSection(root, true);
    }

    applyModifiers(options);
}

Morphology::Morphology(const morphio::Morphology& morphology, unsigned int options)
    : _counter(0)
    , _soma(std::make_shared<Soma>(morphology.soma()))
{
    _cellProperties = std::make_shared<morphio::Property::CellLevel>(
        morphology._properties->_cellLevel);

    for (const morphio::Section& root : morphology.rootSections()) {
        appendRootSection(root, true);
    }

    for (const morphio::MitoSection& root :
        morphology.mitochondria().rootSections()) {
        mitochondria().appendRootSection(root, true);
    }

    applyModifiers(options);
}

/**
   Return false if there is no duplicate point
 **/
bool _checkDuplicatePoint(std::shared_ptr<Section> parent,
    std::shared_ptr<Section> current)
{
    // Weird edge case where parent is empty: skipping it
    if (parent->points().empty())
        return true;

    if (current->points().empty())
        return false;

    if (parent->points()[parent->points().size() - 1] != current->points()[0])
        return false;

    // // As perimeter is optional, it must either be defined for parent and
    // current
    // // or not be defined at all
    // if(parent->perimeters().empty() != current->perimeters().empty())
    //     return false;

    // if(!parent->perimeters().empty() &&
    //    parent->perimeters()[parent->perimeters().size()-1] !=
    //    current->perimeters()[0])
    //     return false;

    return true;
}

std::shared_ptr<Section> Morphology::appendRootSection(
    const morphio::Section& section, bool recursive)
{
    std::shared_ptr<Section> ptr(new Section(this, _counter, section),
        friendDtorForSharedPtr);
    _register(ptr);
    _rootSections.push_back(ptr);

    bool emptySection = ptr->points().empty();
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(ptr));

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Morphology::appendRootSection(
    std::shared_ptr<Section> section, bool recursive)
{
    std::shared_ptr<Section> section_copy(new Section(this, _counter, *section),
        friendDtorForSharedPtr);
    _register(section_copy);
    _rootSections.push_back(section_copy);

    bool emptySection = section_copy->points().empty();
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(section_copy));

    if (recursive) {
        for (const auto child : section->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

std::shared_ptr<Section> Morphology::appendRootSection(
    const Property::PointLevel& pointProperties, SectionType type)
{
    std::shared_ptr<Section> ptr(new Section(this, _counter, type,
                                     pointProperties),
        friendDtorForSharedPtr);
    _register(ptr);
    _rootSections.push_back(ptr);

    bool emptySection = ptr->points().empty();
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(ptr));

    return ptr;
}

void friendDtorForSharedPtr(Section* section)
{
    delete section;
}

uint32_t Morphology::_register(std::shared_ptr<Section> section)
{
    if (_sections.count(section->id()))
        LBTHROW(SectionBuilderError("Section already exists"));
    _counter = std::max(_counter, section->id()) + 1;

    _sections[section->id()] = section;
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

const std::vector<Property::Annotation> Morphology::annotations() const
{
    return _annotations;
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

const std::map<uint32_t, std::shared_ptr<Section>> Morphology::sections() const
{
    return _sections;
}

void eraseByValue(std::vector<std::shared_ptr<Section>>& vec,
    std::shared_ptr<Section> section)
{
    vec.erase(std::remove(vec.begin(), vec.end(), section), vec.end());
}

void Morphology::deleteSection(std::shared_ptr<Section> section, bool recursive)

{
    if (!section)
        return;
    int id = section->id();

    if (recursive) {
        // The deletion must start by the furthest leaves, otherwise you may cut
        // the topology and forget to remove some sections
        std::vector<std::shared_ptr<Section>> ids;
        for (auto it = section->breadth_begin(); it != breadth_end(); ++it) {
            ids.push_back(*it);
        }

        for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
            auto child = *it;
            {
                deleteSection(child, false);
            }
        }
    } else {
        for (auto child : section->children()) {
            // Re-link children to their "grand-parent"
            _parent[child->id()] = _parent[id];
            _children[_parent[id]].push_back(child);
            if (section->isRoot())
                _rootSections.push_back(child);
        }

        eraseByValue(_rootSections, section);
        eraseByValue(_children[_parent[id]], section);
        _children.erase(id);
        _parent.erase(id);
        _sections.erase(id);
    }
}

void _appendProperties(Property::PointLevel& to,
    const Property::PointLevel& from, int offset = 0)
{
    Property::_appendVector(to._points, from._points, offset);
    Property::_appendVector(to._diameters, from._diameters, offset);

    if (!from._perimeters.empty())
        Property::_appendVector(to._perimeters, from._perimeters, offset);
}

void Morphology::sanitize()
{
    sanitize(morphio::plugin::DebugInfo());
}

void Morphology::sanitize(const morphio::plugin::DebugInfo& debugInfo)
{
    morphio::plugin::ErrorMessages err(debugInfo._filename);

    for (auto it = depth_begin(); it != depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        int parentId = section->isRoot() ? -1 : section->parent()->id();
        int sectionId = section->id();

        if (section->isRoot())
            continue;

        if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !_checkDuplicatePoint(section->parent(), section))
            LBERROR(Warning::WRONG_DUPLICATE,
                err.WARNING_WRONG_DUPLICATE(section, section->parent()));

        auto parent = section->parent();
        bool isUnifurcation = parent->children().size() == 1;

        // This "if" condition ensures that "unifurcations" (ie. successive
        // sections with only 1 child) get merged together into a bigger section
        if (isUnifurcation) {
            LBERROR(Warning::ONLY_CHILD,
                err.WARNING_ONLY_CHILD(debugInfo, parentId, sectionId));
            bool duplicate = _checkDuplicatePoint(section->parent(), section);

            addAnnotation(morphio::Property::Annotation(morphio::AnnotationType::SINGLE_CHILD, sectionId,
                section->properties(), "", debugInfo.getLineNumber(parentId)));

            morphio::Property::_appendVector(parent->points(),
                section->points(),
                duplicate ? 1 : 0);

            morphio::Property::_appendVector(parent->diameters(),
                section->diameters(),
                duplicate ? 1 : 0);

            if (!parent->perimeters().empty())
                morphio::Property::_appendVector(parent->perimeters(),
                    section->perimeters(),
                    duplicate ? 1 : 0);

            deleteSection(section, false);
        }
    }
}

const Property::Properties Morphology::buildReadOnly() const
{
    using std::setw;
    int sectionIdOnDisk = 0;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties;

    if (_cellProperties) {
        properties._cellLevel = *_cellProperties;
        properties._cellLevel._somaType = _soma->type();
    }
    _appendProperties(properties._somaLevel, _soma->_pointProperties);

    for (auto it = depth_begin(); it != depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        int parentId = section->isRoot() ? -1 : section->parent()->id();
        int sectionId = section->id();
        int parentOnDisk = (section->isRoot() ? -1 : newIds[parentId]);

        int start = properties._pointLevel._points.size();
        properties._sectionLevel._sections.push_back({start, parentOnDisk});
        properties._sectionLevel._sectionTypes.push_back(section->type());
        newIds[sectionId] = sectionIdOnDisk++;
        _appendProperties(properties._pointLevel, section->_pointProperties);
    }

    mitochondria()._buildMitochondria(properties);
    properties._annotations = annotations();
    return properties;
}

const std::shared_ptr<Section> Morphology::section(uint32_t id) const
{
    return _sections.at(id);
}

depth_iterator Morphology::depth_begin() const
{
    return depth_iterator(*this);
}

depth_iterator Morphology::depth_end() const
{
    return depth_iterator();
}

breadth_iterator Morphology::breadth_begin() const
{
    return breadth_iterator(*this);
}

breadth_iterator Morphology::breadth_end() const
{
    return breadth_iterator();
}

void Morphology::applyModifiers(unsigned int modifierFlags)
{
    if (modifierFlags & NO_DUPLICATES & TWO_POINTS_SECTIONS)
        LBTHROW(SectionBuilderError(
            _err.ERROR_UNCOMPATIBLE_FLAGS(NO_DUPLICATES, TWO_POINTS_SECTIONS)));

    if (modifierFlags & SOMA_SPHERE)
        modifiers::soma_sphere(*this);

    if (modifierFlags & NO_DUPLICATES)
        modifiers::no_duplicate_point(*this);

    if (modifierFlags & TWO_POINTS_SECTIONS)
        modifiers::two_points_sections(*this);

    if (modifierFlags & NRN_ORDER)
        modifiers::nrn_order(*this);
}

void Morphology::write(const std::string& filename)
{
    const size_t pos = filename.find_last_of(".");
    assert(pos != std::string::npos);

    std::string extension;

    morphio::mut::Morphology clean(*this);
    clean.sanitize();

    for (auto& c : filename.substr(pos))
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

bool Morphology::_compare(const Morphology& other, bool verbose) const
{
    if (*(this->_cellProperties) != *(other._cellProperties)) {
        if (verbose)
            std::cout << "Properties differ" << std::endl;
        return false;
    }

    if (this->rootSections().size() != other.rootSections().size()) {
        if (verbose)
            std::cout << "Different number of root sections" << std::endl;
        return false;
    }

    for (unsigned int i = 0; i < this->rootSections().size(); ++i)
        if (*(this->rootSections()[i]) != *(other.rootSections()[i])) {
            if (verbose)
                std::cout << "Sections differ" << std::endl;
            return false;
        }

    return true;
}

bool Morphology::operator==(const Morphology& other) const
{
    return _compare(other, false);
}

bool Morphology::operator!=(const Morphology& other) const
{
    return !_compare(other, false);
}

} // end namespace mut
} // end namespace morphio
