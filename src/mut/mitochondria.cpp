#include <queue>  // std::queue

#include <morphio/mut/mitochondria.h>
#include <morphio/mut/writers.h>


#include "../shared_utils.hpp"

namespace morphio {
namespace mut {

Mitochondria::MitoSectionP Mitochondria::appendRootSection(const morphio::MitoSection& section_,
                                                           bool recursive) {
    const auto ptr = std::make_shared<MitoSection>(this, _counter, section_);
    _register(ptr);
    _rootSections.push_back(ptr);

    if (recursive) {
        for (const auto& child : section_.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

Mitochondria::MitoSectionP Mitochondria::appendRootSection(const MitoSectionP& section_,
                                                           bool recursive) {
    const auto section_copy = std::make_shared<MitoSection>(this, _counter, *section_);
    _register(section_copy);
    _rootSections.push_back(section_copy);

    if (recursive) {
        for (const auto& child : section_->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

Mitochondria::MitoSectionP Mitochondria::appendRootSection(
    const Property::MitochondriaPointLevel& pointProperties) {
    const auto ptr = std::make_shared<MitoSection>(this, _counter, pointProperties);
    _register(ptr);
    _rootSections.push_back(ptr);

    return ptr;
}

static void _appendMitoProperties(Property::MitochondriaPointLevel& to,
                                  const Property::MitochondriaPointLevel& from,
                                  int offset = 0) {
    _appendVector(to._sectionIds, from._sectionIds, offset);
    _appendVector(to._relativePathLengths, from._relativePathLengths, offset);
    _appendVector(to._diameters, from._diameters, offset);
}

const std::vector<Mitochondria::MitoSectionP>& Mitochondria::children(
    const MitoSectionP& section_) const {
    const auto it = _children.find(section_->id());
    if (it == _children.end()) {
        static std::vector<Mitochondria::MitoSectionP> empty;
        return empty;
    }
    return it->second;
}

const Mitochondria::MitoSectionP& Mitochondria::parent(const MitoSectionP& parent_) const {
    return section(_parent.at(parent_->id()));
}

bool Mitochondria::isRoot(const MitoSectionP& section_) const {
    try {
        parent(section_);
        return false;
    } catch (const std::out_of_range&) {
        return true;
    }
}

const Mitochondria::MitoSectionP& Mitochondria::section(uint32_t id) const {
    return _sections.at(id);
}

void Mitochondria::_buildMitochondria(Property::Properties& properties) const {
    int32_t counter = 0;
    std::map<uint32_t, int32_t> newIds;

    for (std::shared_ptr<MitoSection> mitoStart : _rootSections) {
        std::queue<std::shared_ptr<MitoSection>> q;
        q.push(mitoStart);
        while (!q.empty()) {
            std::shared_ptr<MitoSection> section_ = q.front();
            q.pop();
            bool root = isRoot(section_);
            int32_t parentOnDisk = root ? -1 : newIds[parent(section_)->id()];

            properties._mitochondriaSectionLevel._sections.push_back(
                {static_cast<int>(properties._mitochondriaPointLevel._diameters.size()),
                 parentOnDisk});
            _appendMitoProperties(properties._mitochondriaPointLevel, section_->_mitoPoints);

            newIds[section_->id()] = counter++;

            for (auto child : children(section_))
                q.push(child);
        }
    }
}

const std::shared_ptr<MitoSection>& Mitochondria::mitoSection(uint32_t id_) const {
    return _sections.at(id_);
}

mito_depth_iterator Mitochondria::depth_begin(const MitoSectionP& section_) const {
    return mito_depth_iterator(section_);
}

mito_depth_iterator Mitochondria::depth_end() const {
    return mito_depth_iterator();
}

mito_breadth_iterator Mitochondria::breadth_begin(const MitoSectionP& section_) const {
    return mito_breadth_iterator(section_);
}

mito_breadth_iterator Mitochondria::breadth_end() const {
    return mito_breadth_iterator();
}

mito_upstream_iterator Mitochondria::upstream_begin(const MitoSectionP& section_) const {
    return mito_upstream_iterator(section_);
}

mito_upstream_iterator Mitochondria::upstream_end() const {
    return mito_upstream_iterator();
}

uint32_t Mitochondria::_register(const MitoSectionP& section_) {
    if (_sections.count(section_->id()))
        throw SectionBuilderError("Section already exists");
    _counter = std::max(_counter, section_->id()) + 1;

    _sections[section_->id()] = section_;
    return section_->id();
}

}  // namespace mut
}  // namespace morphio
