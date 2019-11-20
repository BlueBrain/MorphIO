#include <assert.h>

#include <sstream>
#include <string>

#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/shared_utils.tpp>
#include <morphio/soma.h>
#include <morphio/tools.h>

namespace morphio {
namespace mut {

void _appendProperties(Property::PointLevel& to,
    const Property::PointLevel& from, int offset);

using morphio::readers::ErrorMessages;
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

    for (const std::shared_ptr<Section>& root : morphology.rootSections()) {
        appendRootSection(root, true);
    }

    for (const std::shared_ptr<MitoSection>& root :
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

    _annotations = morphology.annotations();
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
bool _checkDuplicatePoint(const std::shared_ptr<Section>& parent,
    const std::shared_ptr<Section>& current)
{
    // Weird edge case where parent is empty: skipping it
    if (parent->points().empty())
        return true;

    if (current->points().empty())
        return false;

    if (parent->points().back() != current->points().front())
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
    const morphio::Section& section_, bool recursive)
{
    const std::shared_ptr<Section> ptr(new Section(this, _counter, section_));
    _register(ptr);
    _rootSections.push_back(ptr);

    const bool emptySection = ptr->points().empty();
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(ptr));

    if (recursive) {
        for (const auto& child : section_.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Morphology::appendRootSection(
    const std::shared_ptr<Section>& section_, bool recursive)
{
    const std::shared_ptr<Section> section_copy(new Section(this, _counter, *section_));
    _register(section_copy);
    _rootSections.push_back(section_copy);

    const bool emptySection = section_copy->points().empty();
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(section_copy));

    if (recursive) {
        for (const auto& child : section_->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

std::shared_ptr<Section> Morphology::appendRootSection(
    const Property::PointLevel& pointProperties, SectionType type)
{
    const std::shared_ptr<Section> ptr(new Section(this, _counter, type,
        pointProperties));
    _register(ptr);
    _rootSections.push_back(ptr);

    bool emptySection = ptr->points().empty();
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(ptr));

    return ptr;
}

uint32_t Morphology::_register(const std::shared_ptr<Section>& section_)
{
    if (_sections.count(section_->id()))
        LBTHROW(SectionBuilderError("Section already exists"));
    _counter = std::max(_counter, section_->id()) + 1;

    _sections[section_->id()] = section_;
    return section_->id();
}

Morphology::~Morphology()
{
    auto roots = _rootSections; // Need to iterate on a copy
    for (const auto& root : roots) {
        deleteSection(root, true);
    }
}

static void eraseByValue(std::vector<std::shared_ptr<Section>>& vec,
    const std::shared_ptr<Section>& section)
{
    vec.erase(std::remove(vec.begin(), vec.end(), section), vec.end());
}

void Morphology::deleteSection(const std::shared_ptr<Section>& section_, bool recursive)

{
    if (!section_)
        return;
    unsigned int id = section_->id();

    if (recursive) {
        // The deletion must start by the furthest leaves, otherwise you may cut
        // the topology and forget to remove some sections
        std::vector<std::shared_ptr<Section>> ids;
        std::copy(section_->breadth_begin(), breadth_end(),
            std::back_inserter(ids));

        for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
            deleteSection(*it, false);
        }
    } else {
        for (auto& child : section_->children()) {
            // Re-link children to their "grand-parent"
            _parent[child->id()] = _parent[id];
            _children[_parent[id]].push_back(child);
            if (section_->isRoot())
                _rootSections.push_back(child);
        }

        eraseByValue(_rootSections, section_);
        eraseByValue(_children[_parent[id]], section_);
        _children.erase(id);
        _parent.erase(id);
        _sections.erase(id);
    }
}


void _appendProperties(Property::PointLevel& to,
    const Property::PointLevel& from, int offset = 0)
{
    _appendVector(to._points, from._points, offset);
    _appendVector(to._diameters, from._diameters, offset);

    if (!from._perimeters.empty())
        _appendVector(to._perimeters, from._perimeters, offset);
}

void Morphology::sanitize()
{
    sanitize(morphio::readers::DebugInfo());
}

void Morphology::sanitize(const morphio::readers::DebugInfo& debugInfo)
{
    morphio::readers::ErrorMessages err(debugInfo._filename);

    auto it = depth_begin();
    while (it != depth_end()) {
        std::shared_ptr<Section> section_ = *it;

        // Incrementing iterator here before we potentially delete the section
        ++it;
        unsigned int sectionId = section_->id();

        if (section_->isRoot())
            continue;

        unsigned int parentId = section_->parent()->id();

        if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !_checkDuplicatePoint(section_->parent(), section_))
            LBERROR(Warning::WRONG_DUPLICATE,
                err.WARNING_WRONG_DUPLICATE(section_, section_->parent()));

        auto parent = section_->parent();
        bool isUnifurcation = parent->children().size() == 1;

        // This "if" condition ensures that "unifurcations" (ie. successive
        // sections with only 1 child) get merged together into a bigger section
        if (isUnifurcation) {
            LBERROR(Warning::ONLY_CHILD,
                err.WARNING_ONLY_CHILD(debugInfo, parentId, sectionId));
            bool duplicate = _checkDuplicatePoint(section_->parent(), section_);

            addAnnotation(morphio::Property::Annotation(morphio::AnnotationType::SINGLE_CHILD, sectionId,
                section_->properties(), "", debugInfo.getLineNumber(parentId)));

            morphio::_appendVector(parent->points(),
                section_->points(),
                duplicate ? 1 : 0);

            morphio::_appendVector(parent->diameters(),
                section_->diameters(),
                duplicate ? 1 : 0);

            if (!parent->perimeters().empty())
                morphio::_appendVector(parent->perimeters(),
                    section_->perimeters(),
                    duplicate ? 1 : 0);

            deleteSection(section_, false);
        }
    }
}

Property::Properties Morphology::buildReadOnly() const
{
    using std::setw;
    int sectionIdOnDisk = 0;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties {};

    if (_cellProperties) {
        properties._cellLevel = *_cellProperties;
        properties._cellLevel._somaType = _soma->type();
    }
    _appendProperties(properties._somaLevel, _soma->_pointProperties);

    for (auto it = depth_begin(); it != depth_end(); ++it) {
        const std::shared_ptr<Section>& section_ = *it;
        unsigned int sectionId = section_->id();
        int parentOnDisk = (section_->isRoot() ? -1 : newIds[section_->parent()->id()]);

        auto start = static_cast<int>(properties._pointLevel._points.size());
        properties._sectionLevel._sections.push_back({start, parentOnDisk});
        properties._sectionLevel._sectionTypes.push_back(section_->type());
        newIds[sectionId] = sectionIdOnDisk++;
        _appendProperties(properties._pointLevel, section_->_pointProperties);
    }

    mitochondria()._buildMitochondria(properties);
    properties._annotations = annotations();
    return properties;
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

    for (const auto& root: clean.rootSections()) {
        if(root->points().size() < 2)
            throw morphio::SectionBuilderError("Root sections must have at least 2 points");
    }

    for (char c : filename.substr(pos))
        extension += my_tolower(c);

    if (extension == ".h5")
        writer::h5(clean, filename);
    else if (extension == ".asc")
        writer::asc(clean, filename);
    else if (extension == ".swc")
        writer::swc(clean, filename);
    else
        LBTHROW(UnknownFileType(_err.ERROR_WRONG_EXTENSION(filename)));
}

} // end namespace mut
} // end namespace morphio
