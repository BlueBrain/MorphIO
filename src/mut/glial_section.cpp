#include <stack>

#include <morphio/errorMessages.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/glial_section.h>
#include <morphio/tools.h>



namespace morphio {

extern template class Node<CellFamily::GLIA>;

namespace mut {
using morphio::readers::ErrorMessages;

using glial_depth_iterator = depth_iterator_t<std::shared_ptr<GlialSection>>;
using glial_breadth_iterator = breadth_iterator_t<std::shared_ptr<GlialSection>>;
using glial_upstream_iterator = upstream_iterator_t<std::shared_ptr<GlialSection>>;

static inline bool _emptySection(const std::shared_ptr<GlialSection>& section) {
    return section->points().empty();
}

GlialSection::GlialSection(GlialCell* glialCell,
                 unsigned int id_,
                 GlialSectionType type_,
                 const Property::PointLevel& pointProperties)
    : _morphology(glialCell)
    , _pointProperties(pointProperties)
    , _id(id_)
    , _sectionType(type_) {}

GlialSection::GlialSection(GlialCell* glialCell, unsigned int id_, const morphio::GlialSection& section_)
    : GlialSection(glialCell,
              id_,
              section_.type(),
              Property::PointLevel(section_._properties->_pointLevel, section_._range)) {}

GlialSection::GlialSection(GlialCell* glialCell, unsigned int id_, const GlialSection& section_)
    : _morphology(glialCell)
    , _pointProperties(section_._pointProperties)
    , _id(id_)
    , _sectionType(section_._sectionType) {}

const std::shared_ptr<GlialSection>& GlialSection::parent() const {
    return _morphology->_sections.at(_morphology->_parent.at(id()));
}

bool GlialSection::isRoot() const {
    const auto parentId = _morphology->_parent.find(id());
    if (parentId != _morphology->_parent.end()) {
        return _morphology->_sections.find(parentId->second) == _morphology->_sections.end();
    }
    return true;
}

const std::vector<std::shared_ptr<GlialSection>>& GlialSection::children() const {
    const auto it = _morphology->_children.find(id());
    if (it == _morphology->_children.end()) {
        static std::vector<std::shared_ptr<GlialSection>> empty;
        return empty;
    }
    return it->second;
}

glial_depth_iterator GlialSection::depth_begin() const {
    return glial_depth_iterator(const_cast<GlialSection*>(this)->shared_from_this());
}

glial_depth_iterator GlialSection::depth_end() const {
    return glial_depth_iterator();
}

glial_breadth_iterator GlialSection::breadth_begin() const {
    return glial_breadth_iterator(const_cast<GlialSection*>(this)->shared_from_this());
}

glial_breadth_iterator GlialSection::breadth_end() const {
    return glial_breadth_iterator();
}

glial_upstream_iterator GlialSection::upstream_begin() const {
    return glial_upstream_iterator(const_cast<GlialSection*>(this)->shared_from_this());
}

glial_upstream_iterator GlialSection::upstream_end() const {
    return glial_upstream_iterator();
}

static std::ostream& operator<<(std::ostream& os, const GlialSection& section) {
    ::operator<<(os, section);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<GlialSection>& sectionPtr) {
    os << *sectionPtr;
    return os;
}

std::shared_ptr<GlialSection> GlialSection::appendSection(const std::shared_ptr<GlialSection>& original_section,
                                                bool recursive) {
    const std::shared_ptr<GlialSection> ptr(
        new GlialSection(_morphology, _morphology->_counter, *original_section));
    unsigned int parentId = id();
    uint32_t childId = _morphology->_register(ptr);
    auto& _sections = _morphology->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   _morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId])) {
        printError(Warning::WRONG_DUPLICATE,
                   _morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                             _sections.at(parentId)));
    }

    _morphology->_parent[childId] = parentId;
    _morphology->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<GlialSection> GlialSection::appendSection(const morphio::GlialSection& section, bool recursive) {
    const std::shared_ptr<GlialSection> ptr(new GlialSection(_morphology, _morphology->_counter, section));
    unsigned int parentId = id();
    uint32_t childId = _morphology->_register(ptr);
    auto& _sections = _morphology->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   _morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        printError(Warning::WRONG_DUPLICATE,
                   _morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                             _sections.at(parentId)));

    _morphology->_parent[childId] = parentId;
    _morphology->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<GlialSection> GlialSection::appendSection(const Property::PointLevel& pointProperties,
                                                GlialSectionType sectionType) {
    unsigned int parentId = id();

    auto& _sections = _morphology->_sections;
    if (sectionType == GlialSectionType::UNDEFINED)
        sectionType = type();

    if (sectionType == GlialSectionType::SOMA)
        throw morphio::SectionBuilderError("Cannot create section with type soma");

    std::shared_ptr<GlialSection> ptr(
        new GlialSection(_morphology, _morphology->_counter, sectionType, pointProperties));

    uint32_t childId = _morphology->_register(ptr);

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   _morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        printError(Warning::WRONG_DUPLICATE,
                   _morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                             _sections[parentId]));

    _morphology->_parent[childId] = parentId;
    _morphology->_children[parentId].push_back(ptr);
    return ptr;
}

}  // end namespace mut
}  // end namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::mut::GlialSection& section) {
    auto points = section.points();
    if (points.empty()) {
        os << "GlialSection(id=" << section.id() << ", points=[])";
    } else {
        os << "GlialSection(id=" << section.id() << ", points=[(" << points[0] << "),..., (";
        os << points[points.size() - 1] << ")])";
    }

    return os;
}
