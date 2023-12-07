/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <algorithm>  // std::find_if
#include <cctype>    // std::tolower
#include <iterator>  // std::back_inserter
#include <string>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mitochondria.h>
#include <morphio/mut/endoplasmic_reticulum.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/soma.h>

#include "../shared_utils.hpp"

namespace {
using SectionP = std::shared_ptr<morphio::mut::Section>;

void insertSectionsBeforeSection(std::vector<SectionP>& sections_to_update,
                                 const std::vector<SectionP>& sections,
                                 const SectionP& target_section) {
    auto is_equal = [&target_section](const SectionP& section) {
        return (section->id() == target_section->id()) && section->hasSameShape(*target_section);
    };

    const auto it = std::find_if(sections_to_update.begin(), sections_to_update.end(), is_equal);

    // Note that the section with section_id is not removed at this step
    sections_to_update.insert(it, sections.begin(), sections.end());
}

void appendProperties(morphio::Property::PointLevel& to,
                      const morphio::Property::PointLevel& from,
                      int offset = 0) {
    morphio::_appendVector(to._points, from._points, offset);
    morphio::_appendVector(to._diameters, from._diameters, offset);

    if (!from._perimeters.empty()) {
        morphio::_appendVector(to._perimeters, from._perimeters, offset);
    }
}
}  // namespace

namespace morphio {
namespace mut {

Morphology::Morphology(const std::string& uri, unsigned int options)
    : Morphology(morphio::Morphology(uri, options)) {}

Morphology::Morphology(const HighFive::Group& group, unsigned int options)
    : Morphology(morphio::Morphology(group, options)) {}

Morphology::Morphology(const morphio::mut::Morphology& morphology, unsigned int options)
    : _soma(std::make_shared<Soma>(*morphology.soma()))
    , _cellProperties(std::make_shared<morphio::Property::CellLevel>(*morphology._cellProperties))
    , _endoplasmicReticulum(morphology.endoplasmicReticulum())
    , _dendriticSpineLevel(morphology._dendriticSpineLevel) {
    for (const std::shared_ptr<Section>& root : morphology.rootSections()) {
        appendRootSection(root, true);
    }

    for (const std::shared_ptr<MitoSection>& root : morphology.mitochondria().rootSections()) {
        mitochondria().appendRootSection(root, true);
    }

    applyModifiers(options);
}

Morphology::Morphology(const morphio::Morphology& morphology, unsigned int options)
    : _soma(std::make_shared<Soma>(morphology.soma()))
    , _cellProperties(
          std::make_shared<morphio::Property::CellLevel>(morphology.properties_->_cellLevel))
    , _endoplasmicReticulum(morphology.endoplasmicReticulum())
    , _dendriticSpineLevel(morphology.properties_->_dendriticSpineLevel) {
    for (const morphio::Section& root : morphology.rootSections()) {
        appendRootSection(root, true);
    }

    for (const morphio::MitoSection& root : morphology.mitochondria().rootSections()) {
        mitochondria().appendRootSection(root, true);
    }

    applyModifiers(options);
}

/**
   Return false if there is no duplicate point
 **/
bool _checkDuplicatePoint(const SectionP& parent, const SectionP& current) {
    // Weird edge case where parent is empty: skipping it
    if (parent->points().empty()) {
        return true;
    } else if (current->points().empty()) {
        return false;
    } else if (parent->points().back() != current->points().front()) {
        return false;
    }

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

std::shared_ptr<Section> Morphology::appendRootSection(const morphio::Section& section_,
                                                       bool recursive) {
    std::shared_ptr<Section> ptr(new Section(this, _counter, section_));
    _register(ptr);
    _rootSections.push_back(ptr);

    const bool emptySection = ptr->points().empty();
    if (emptySection) {
        printError(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(ptr));
    }

    if (recursive) {
        for (const auto& child : section_.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<Section> Morphology::appendRootSection(const std::shared_ptr<Section>& section_,
                                                       bool recursive) {
    std::shared_ptr<Section> section_copy(new Section(this, _counter, *section_));
    _register(section_copy);
    _rootSections.push_back(section_copy);
    const bool emptySection = section_copy->points().empty();
    if (emptySection) {
        printError(Warning::APPENDING_EMPTY_SECTION,
                   _err.WARNING_APPENDING_EMPTY_SECTION(section_copy));
    }

    if (recursive) {
        for (const auto& child : section_->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

std::shared_ptr<Section> Morphology::appendRootSection(const Property::PointLevel& pointProperties,
                                                       SectionType type) {
    std::shared_ptr<Section> ptr(new Section(this, _counter, type, pointProperties));
    _register(ptr);
    _rootSections.push_back(ptr);

    bool emptySection = ptr->points().empty();
    if (emptySection) {
        printError(Warning::APPENDING_EMPTY_SECTION, _err.WARNING_APPENDING_EMPTY_SECTION(ptr));
    }

    return ptr;
}

uint32_t Morphology::_register(const std::shared_ptr<Section>& section_) {
    if (_sections.count(section_->id()) > 0) {
        throw SectionBuilderError("Section already exists");
    }
    _counter = std::max(_counter, section_->id()) + 1;
    _sections[section_->id()] = section_;
    return section_->id();
}

Morphology::~Morphology() {
    auto roots = _rootSections;  // Need to iterate on a copy
    for (const auto& root : roots) {
        deleteSection(root, true);
    }
}

void Morphology::eraseByValue(std::vector<std::shared_ptr<Section>>& vec,
                              const std::shared_ptr<Section> section) {
    if (section->morphology_ == this) {
        section->morphology_ = nullptr;
        section->id_ = 0xffffffff;
    }
    vec.erase(std::remove(vec.begin(), vec.end(), section), vec.end());
}


void Morphology::deleteSection(const std::shared_ptr<Section> section_, bool recursive) {
    if (!section_) {
        return;
    }

    if (recursive) {
        // The deletion must start by the furthest leaves, otherwise you may cut
        // the topology and forget to remove some sections
        std::vector<std::shared_ptr<Section>> ids;
        std::copy(section_->breadth_begin(), breadth_end(), std::back_inserter(ids));

        for (auto it = ids.rbegin(); it != ids.rend(); ++it) {
            deleteSection(*it, false);
        }
    } else {
        const auto section_id = section_->id();

        // Careful not to use a reference here or you will face reference invalidation problem
        // with vector resize
        const auto children = section_->children();

        if (section_->isRoot()) {
            // put root section's children in its place
            insertSectionsBeforeSection(_rootSections, children, section_);
            eraseByValue(_rootSections, section_);
        } else {
            // set grandparent as section children's parent
            for (const auto& child : children) {
                _parent[child->id()] = _parent[section_id];
            }

            auto& children_of_parent = _children[_parent[section_id]];

            // put grandchildren at the position of the deleted section
            insertSectionsBeforeSection(children_of_parent, children, section_);
            eraseByValue(children_of_parent, section_);
        }

        // remove section id from connectivity and section lists
        _children.erase(section_id);
        _parent.erase(section_id);
        _sections.erase(section_id);
    }
}

void Morphology::removeUnifurcations() {
    removeUnifurcations(morphio::readers::DebugInfo());
}

void Morphology::removeUnifurcations(const morphio::readers::DebugInfo& debugInfo) {
    morphio::readers::ErrorMessages err(debugInfo._filename);

    auto it = depth_begin();
    while (it != depth_end()) {
        std::shared_ptr<Section> section_ = *it;

        // Incrementing iterator here before we potentially delete the section
        ++it;
        unsigned int sectionId = section_->id();

        if (section_->isRoot()) {
            continue;
        }

        unsigned int parentId = section_->parent()->id();

        if (!morphio::readers::ErrorMessages::isIgnored(Warning::WRONG_DUPLICATE) &&
            !_checkDuplicatePoint(section_->parent(), section_)) {
            printError(Warning::WRONG_DUPLICATE,
                       err.WARNING_WRONG_DUPLICATE(section_, section_->parent()));
        }

        auto parent = section_->parent();
        bool isUnifurcation = parent->children().size() == 1;

        // This "if" condition ensures that "unifurcations" (ie. successive
        // sections with only 1 child) get merged together into a bigger section
        if (isUnifurcation) {
            printError(Warning::ONLY_CHILD, err.WARNING_ONLY_CHILD(debugInfo, parentId, sectionId));
            bool duplicate = _checkDuplicatePoint(section_->parent(), section_);

            addAnnotation(morphio::Property::Annotation(morphio::AnnotationType::SINGLE_CHILD,
                                                        sectionId,
                                                        section_->properties(),
                                                        "",
                                                        debugInfo.getLineNumber(parentId)));

            morphio::_appendVector(parent->points(), section_->points(), duplicate ? 1 : 0);

            morphio::_appendVector(parent->diameters(), section_->diameters(), duplicate ? 1 : 0);

            if (!parent->perimeters().empty()) {
                morphio::_appendVector(parent->perimeters(),
                                       section_->perimeters(),
                                       duplicate ? 1 : 0);
            }

            deleteSection(section_, false);
        }
    }
}

Property::Properties Morphology::buildReadOnly() const {
    int sectionIdOnDisk = 0;
    std::map<uint32_t, int32_t> newIds;
    Property::Properties properties{};

    properties._cellLevel = *_cellProperties;
    properties._cellLevel._somaType = _soma->type();
    appendProperties(properties._somaLevel, _soma->point_properties_);

    for (auto it = depth_begin(); it != depth_end(); ++it) {
        const std::shared_ptr<Section>& section = *it;
        unsigned int sectionId = section->id();
        int parentOnDisk = (section->isRoot() ? -1 : newIds[section->parent()->id()]);

        auto start = static_cast<int>(properties._pointLevel._points.size());
        properties._sectionLevel._sections.push_back({start, parentOnDisk});
        properties._sectionLevel._sectionTypes.push_back(section->type());
        newIds[sectionId] = sectionIdOnDisk++;
        appendProperties(properties._pointLevel, section->point_properties_);
    }

    mitochondria()._buildMitochondria(properties);
    properties._endoplasmicReticulumLevel = endoplasmicReticulum().buildReadOnly();
    properties._dendriticSpineLevel = _dendriticSpineLevel;

    return properties;
}

depth_iterator Morphology::depth_begin() const {
    return depth_iterator(*this);
}

depth_iterator Morphology::depth_end() const {
    return depth_iterator();
}

breadth_iterator Morphology::breadth_begin() const {
    return breadth_iterator(*this);
}

breadth_iterator Morphology::breadth_end() const {
    return breadth_iterator();
}

void Morphology::applyModifiers(unsigned int modifierFlags) {
    if (modifierFlags & NO_DUPLICATES & TWO_POINTS_SECTIONS) {
        throw SectionBuilderError(
            _err.ERROR_UNCOMPATIBLE_FLAGS(NO_DUPLICATES, TWO_POINTS_SECTIONS));
    }

    if (modifierFlags & SOMA_SPHERE) {
        modifiers::soma_sphere(*this);
    }

    if (modifierFlags & NO_DUPLICATES) {
        modifiers::no_duplicate_point(*this);
    }

    if (modifierFlags & TWO_POINTS_SECTIONS) {
        modifiers::two_points_sections(*this);
    }

    if (modifierFlags & NRN_ORDER) {
        modifiers::nrn_order(*this);
    }
}

std::unordered_map<int, std::vector<unsigned int>> Morphology::connectivity() {
    std::unordered_map<int, std::vector<unsigned int>> connectivity;

    const auto& roots = rootSections();
    connectivity[-1].reserve(roots.size());
    std::transform(roots.begin(),
                   roots.end(),
                   std::back_inserter(connectivity[-1]),
                   [](const std::shared_ptr<Section>& section) { return section->id(); });

    for (const auto& kv : _children) {
        auto& nodeEdges = connectivity[static_cast<int>(kv.first)];
        nodeEdges.reserve(kv.second.size());
        std::transform(kv.second.begin(),
                       kv.second.end(),
                       std::back_inserter(nodeEdges),
                       [](const std::shared_ptr<Section>& section) { return section->id(); });
    }

    return connectivity;
}

void Morphology::write(const std::string& filename) const {
    for (const auto& root : rootSections()) {
        if (root->points().size() < 2) {
            throw morphio::SectionBuilderError("Root sections must have at least 2 points");
        }
    }

    const size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) {
        throw UnknownFileType("Missing file extension.");
    }

    std::string extension;
    for (char c : filename.substr(pos)) {
        extension += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (extension == ".h5") {
        writer::h5(*this, filename);
    } else if (extension == ".asc") {
        writer::asc(*this, filename);
    } else if (extension == ".swc") {
        writer::swc(*this, filename);
    } else {
        throw UnknownFileType(_err.ERROR_WRONG_EXTENSION(filename));
    }
}

}  // end namespace mut
}  // end namespace morphio
