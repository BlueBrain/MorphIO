#include <stack>

#include <morphio/errorMessages.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>

namespace morphio {
namespace mut {
using morphio::plugin::ErrorMessages;

Section::Section(Morphology* morphology, int id, SectionType type,
    const Property::PointLevel& pointProperties)
    : _morphology(morphology)
    , _pointProperties(pointProperties)
    , _id(id)
    , _sectionType(type)
{
}

Section::Section(Morphology* morphology, int id,
    const morphio::Section& section)
    : Section(morphology, id, section.type(),
          Property::PointLevel(section._properties->_pointLevel,
              section._range))
{
}

Section::Section(Morphology* morphology, int id, const Section& section)
    : _morphology(morphology)
    , _pointProperties(section._pointProperties)
    , _id(id)
    , _sectionType(section._sectionType)
{
}

const std::shared_ptr<Section> Section::parent() const
{
    return _morphology->_sections.at(_morphology->_parent.at(id()));
}

bool Section::isRoot() const
{
    try {
        parent();
        return false;
    } catch (const std::out_of_range& e) {
        return true;
    }
}

const std::vector<std::shared_ptr<Section>> Section::children() const
{
    try {
        return _morphology->_children.at(id());
    } catch (const std::out_of_range& e) {
        return std::vector<std::shared_ptr<Section>>();
    }
}

depth_iterator Section::depth_begin() const
{
    return depth_iterator(const_cast<Section*>(this)->shared_from_this());
}

depth_iterator Section::depth_end() const
{
    return depth_iterator();
}

breadth_iterator Section::breadth_begin() const
{
    return breadth_iterator(const_cast<Section*>(this)->shared_from_this());
}

breadth_iterator Section::breadth_end() const
{
    return breadth_iterator();
}

upstream_iterator Section::upstream_begin() const
{
    return upstream_iterator(const_cast<Section*>(this)->shared_from_this());
}

upstream_iterator Section::upstream_end() const
{
    return upstream_iterator();
}

std::ostream& operator<<(std::ostream& os, Section& section)
{
    os << "id: " << section.id() << std::endl;
    os << dumpPoints(section.points());
    return os;
}

std::ostream& operator<<(std::ostream& os, std::shared_ptr<Section> sectionPtr)
{
    os << *sectionPtr;
    return os;
}

bool _emptySection(const std::shared_ptr<Section> section)
{
    return section->points().empty();
}
std::shared_ptr<Section> Section::appendSection(
    std::shared_ptr<Section> original_section, bool recursive)
{
    std::shared_ptr<Section> ptr(new Section(_morphology, _morphology->_counter,
                                     *original_section),
        friendDtorForSharedPtr);
    int32_t parentId = id();
    uint32_t childId = _morphology->_register(ptr);
    auto& _sections = _morphology->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection && !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        LBERROR(Warning::WRONG_DUPLICATE,
            _morphology->_err.WARNING_WRONG_DUPLICATE(
                _sections[childId], _sections.at(parentId)));

    _morphology->_parent[childId] = parentId;
    _morphology->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto child : original_section->children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(const morphio::Section& section,
    bool recursive)
{
    std::shared_ptr<Section> ptr(new Section(_morphology, _morphology->_counter,
                                     section),
        friendDtorForSharedPtr);
    int32_t parentId = id();
    uint32_t childId = _morphology->_register(ptr);
    auto& _sections = _morphology->_sections;

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection && !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        LBERROR(Warning::WRONG_DUPLICATE,
            _morphology->_err.WARNING_WRONG_DUPLICATE(
                _sections[childId], _sections.at(parentId)));

    _morphology->_parent[childId] = parentId;
    _morphology->_children[parentId].push_back(ptr);

    if (recursive) {
        for (const auto& child : section.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Section::appendSection(
    const Property::PointLevel& pointProperties, SectionType sectionType)
{
    int32_t parentId = id();

    auto& _sections = _morphology->_sections;
    if (sectionType == SectionType::SECTION_UNDEFINED)
        sectionType = type();

    if (sectionType == SECTION_SOMA)
        LBTHROW(morphio::SectionBuilderError(
            "Cannot create section with type soma"));

    Section* p = new Section(_morphology, _morphology->_counter, sectionType,
        pointProperties);

    std::shared_ptr<Section> ptr(p, friendDtorForSharedPtr);

    uint32_t childId = _morphology->_register(ptr);

    bool emptySection = _emptySection(_sections[childId]);
    if (emptySection)
        LBERROR(Warning::APPENDING_EMPTY_SECTION, _morphology->_err.WARNING_APPENDING_EMPTY_SECTION(_sections[childId]));

    if (!ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) && !emptySection && !_checkDuplicatePoint(_sections[parentId], _sections[childId]))
        LBERROR(Warning::WRONG_DUPLICATE,
            _morphology->_err.WARNING_WRONG_DUPLICATE(_sections[childId],
                _sections[parentId]));

    _morphology->_parent[childId] = parentId;
    _morphology->_children[parentId].push_back(ptr);
    return ptr;
}

bool Section::_compare(const Section& other, bool verbose) const
{
    if (this->type() != other.type()) {
        if (verbose)
            std::cout << "Section type differ" << std::endl;
        return false;
    }

    if (this->points() != other.points()) {
        if (verbose)
            std::cout << "Points differ" << std::endl;
        return false;
    }

    if (this->diameters() != other.diameters()) {
        if (verbose)
            std::cout << "Diameters differ" << std::endl;
        return false;
    }

    if (this->perimeters() != other.perimeters()) {
        if (verbose)
            std::cout << "Perimeters differ" << std::endl;
        return false;
    }

    if (this->children().size() != other.children().size()) {
        if (verbose)
            std::cout << "Different number of children" << std::endl;
        return false;
    }

    for (unsigned int i = 0; i < this->children().size(); ++i)
        if (*(this->children()[i]) != *(other.children()[i])) {
            if (verbose)
                std::cout << "Sections differ" << std::endl;
            return false;
        }

    return true;
}

bool Section::operator==(const Section& other) const
{
    return _compare(other, false);
}

bool Section::operator!=(const Section& other) const
{
    return !_compare(other, false);
}

} // end namespace mut
} // end namespace morphio
