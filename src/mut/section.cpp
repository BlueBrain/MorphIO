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

Section::Section(Morphology* morphology,
                 unsigned int id_,
                 SectionType type_,
                 const Property::PointLevel& pointProperties)
    : _morphology(morphology)
    , _pointProperties(pointProperties)
    , _id(id_)
    , _sectionType(type_) {}

Section::Section(Morphology* morphology, unsigned int id_, const morphio::Section& section_)
    : Section(morphology,
              id_,
              section_.type(),
              Property::PointLevel(section_._properties->_pointLevel, section_._range)) {}

Section::Section(Morphology* morphology, unsigned int id_, const Section& section_)
    : _morphology(morphology)
    , _pointProperties(section_._pointProperties)
    , _id(id_)
    , _sectionType(section_._sectionType) {}

void Section::throwIfNoOwningMorphology() const {
    if (!_morphology) {
        throw std::runtime_error("Section does not belong to a morphology, impossible operation");
    }
}

Morphology* Section::getOwningMorphologyOrThrow() const {
    throwIfNoOwningMorphology();
    return _morphology;
}

const std::shared_ptr<Section>& Section::parent() const {
    const Morphology* morphology = getOwningMorphologyOrThrow();
    return morphology->_sections.at(morphology->_parent.at(id()));
}

bool Section::isRoot() const {
    const Morphology* morphology = getOwningMorphologyOrThrow();
    const auto parentId = morphology->_parent.find(id());
    if (parentId != morphology->_parent.end()) {
        return morphology->_sections.find(parentId->second) == morphology->_sections.end();
    }
    return true;
}

const std::vector<std::shared_ptr<Section>>& Section::children() const {
    const Morphology* morphology = getOwningMorphologyOrThrow();

    const auto it = morphology->_children.find(id());
    if (it == morphology->_children.end()) {
        static std::vector<std::shared_ptr<Section>> empty;
        return empty;
    }
    return it->second;
}

depth_iterator Section::depth_begin() const {
    throwIfNoOwningMorphology();
    return depth_iterator(const_cast<Section*>(this)->shared_from_this());
}

depth_iterator Section::depth_end() const {
    throwIfNoOwningMorphology();
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const {
    throwIfNoOwningMorphology();
    return breadth_iterator(const_cast<Section*>(this)->shared_from_this());
}

breadth_iterator Section::breadth_end() const {
    throwIfNoOwningMorphology();
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const {
    throwIfNoOwningMorphology();
    return upstream_iterator(const_cast<Section*>(this)->shared_from_this());
}

upstream_iterator Section::upstream_end() const {
    throwIfNoOwningMorphology();
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

std::shared_ptr<Section> Section::appendSection(std::shared_ptr<Section> original_section,
                                                bool recursive) {
    Morphology* morphology = getOwningMorphologyOrThrow();

    const std::shared_ptr<Section> ptr(
        new Section(morphology, morphology->_counter, *original_section));
    unsigned int parentId = id();
    uint32_t childId = morphology->_register(ptr);
    auto& _sections = morphology->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId])) {
        printError(Warning::WRONG_DUPLICATE,
                   morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                            _sections.at(parentId)));
    }

    morphology->_parent[childId] = parentId;
    morphology->_children[parentId].push_back(ptr);

    // Careful not to use a reference here or you will face ref invalidation problem with vector
    // resize The argument `original_section` of this function could be a reference to the
    // `_children` array and that reference might be invalidated by this `push_back` (in case
    // `vector` needs to reallocate the array)
    if (recursive) {
        for (auto child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const morphio::Section& section, bool recursive) {
    Morphology* morphology = getOwningMorphologyOrThrow();
    const std::shared_ptr<Section> ptr(new Section(morphology, morphology->_counter, section));
    unsigned int parentId = id();
    uint32_t childId = morphology->_register(ptr);
    auto& _sections = morphology->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        printError(Warning::WRONG_DUPLICATE,
                   morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                            _sections.at(parentId)));

    morphology->_parent[childId] = parentId;
    morphology->_children[parentId].push_back(ptr);

    if (recursive) {
        for (auto child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const Property::PointLevel& pointProperties,
                                                SectionType sectionType) {
    Morphology* morphology = getOwningMorphologyOrThrow();
    unsigned int parentId = id();

    auto& _sections = morphology->_sections;
    if (sectionType == SectionType::SECTION_UNDEFINED)
        sectionType = type();

    if (sectionType == SECTION_SOMA)
        throw morphio::SectionBuilderError("Cannot create section with type soma");

    std::shared_ptr<Section> ptr(
        new Section(morphology, morphology->_counter, sectionType, pointProperties));

    uint32_t childId = morphology->_register(ptr);

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        printError(Warning::APPENDING_EMPTY_SECTION,
                   morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection &&
        !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        printError(Warning::WRONG_DUPLICATE,
                   morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                                                            _sections[parentId]));

    morphology->_parent[childId] = parentId;
    morphology->_children[parentId].push_back(ptr);
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
