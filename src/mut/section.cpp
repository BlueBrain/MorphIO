#include <stack>

#include <morphio/errorMessages.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/tools.h>

namespace morphio {
namespace mut {
using morphio::readers::ErrorMessages;

static inline bool _emptySection(const std::shared_ptr<Section>& section) {
    return section->points().empty();
}

std::shared_ptr<Morphology> Section::_getMorphology() const {
    auto shared = _morphology.lock();
    if (shared)
        return shared;
    else
        throw std::runtime_error(
            "This section (id: " + std::to_string(id()) + ") can no longer be used in the context of the whole morphology "
            "because the Morphology object it belongs to has been deleted");
}

Section::Section(std::weak_ptr<Morphology> morphology,
                 unsigned int id_,
                 SectionType type_,
                 const Property::PointLevel& pointProperties)
    : _morphology(morphology)
    , _pointProperties(pointProperties)
    , _id(id_)
    , _sectionType(type_) {}

Section::Section(std::weak_ptr<Morphology> morphology, unsigned int id_, const morphio::Section& section_)
    : Section(morphology,
              id_,
              section_.type(),
              Property::PointLevel(section_._properties->_pointLevel, section_._range)) {}

Section::Section(std::weak_ptr<Morphology> morphology, unsigned int id_, const Section& section_)
    : _morphology(morphology)
    , _pointProperties(section_._pointProperties)
    , _id(id_)
    , _sectionType(section_._sectionType) {}

const std::shared_ptr<Section>& Section::parent() const {
    auto morph = _getMorphology();
    return morph->_sections.at(morph->_parent.at(id()));
}

bool Section::isRoot() const {
    auto morph = _getMorphology();
    const auto parentId = morph->_parent.find(id());
    if (parentId != morph->_parent.end()) {
        return morph->_sections.find(parentId->second) == morph->_sections.end();
    }
    return true;
}

const std::vector<std::shared_ptr<Section>>& Section::children() const {
    auto morph = _getMorphology();
    const auto it = morph->_children.find(id());
    if (it == morph->_children.end()) {
        static std::vector<std::shared_ptr<Section>> empty;
        return empty;
    }
    return it->second;
}

depth_iterator Section::depth_begin() const {
    return depth_iterator(const_cast<Section*>(this)->shared_from_this());
}

depth_iterator Section::depth_end() const {
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const {
    return breadth_iterator(const_cast<Section*>(this)->shared_from_this());
}

breadth_iterator Section::breadth_end() const {
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const {
    return upstream_iterator(const_cast<Section*>(this)->shared_from_this());
}

upstream_iterator Section::upstream_end() const {
    return upstream_iterator();
}

static std::ostream& operator<<(std::ostream& os, const Section& section) {
    ::operator<<(os, section);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Section>& sectionPtr) {
    os << *sectionPtr;
    return os;
}

std::shared_ptr<Section> Section::appendSection(const std::shared_ptr<Section>& original_section,
                                                bool recursive) {
    auto morph = _getMorphology();
    const std::shared_ptr<Section> ptr(
        new Section(_morphology, morph->_counter, *original_section));
    unsigned int parentId = id();
    uint32_t childId = morph->_register(ptr);
    auto& _sections = morph->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   morph->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId])) {
        printError(Warning::WRONG_DUPLICATE,
                   morph->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                             _sections.at(parentId)));
    }

    morph->_parent[childId] = parentId;
    morph->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const morphio::Section& section, bool recursive) {
    auto morph = _getMorphology();
    const std::shared_ptr<Section> ptr(new Section(_morphology,
                                                   morph->_counter, section));
    unsigned int parentId = id();
    uint32_t childId = morph->_register(ptr);
    auto& _sections = morph->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   morph->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        printError(Warning::WRONG_DUPLICATE,
                   morph->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                             _sections.at(parentId)));

    morph->_parent[childId] = parentId;
    morph->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const Property::PointLevel& pointProperties,
                                                SectionType sectionType) {
    auto morph = _getMorphology();
    unsigned int parentId = id();

    auto& _sections = morph->_sections;
    if (sectionType == SectionType::SECTION_UNDEFINED)
        sectionType = type();

    if (sectionType == SECTION_SOMA)
        throw morphio::SectionBuilderError("Cannot create section with type soma");

    std::shared_ptr<Section> ptr(
        new Section(_morphology, morph->_counter, sectionType, pointProperties));

    uint32_t childId = morph->_register(ptr);

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   morph->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        printError(Warning::WRONG_DUPLICATE,
                   morph->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                             _sections[parentId]));

    morph->_parent[childId] = parentId;
    morph->_children[parentId].push_back(ptr);
    return ptr;
}

}  // end namespace mut
}  // end namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::mut::Section& section) {
    auto points = section.points();
    if (points.empty()) {
        os << "Section(id=" << section.id() << ", points=[])";
    } else {
        os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., (";
        os << points[points.size() - 1] << ")])";
    }

    return os;
}
