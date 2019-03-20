#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/writers.h>
#include <morphio/shared_utils.tpp>

namespace morphio {
namespace mut {

void friendDtorForSharedPtrMito(MitoSection* section)
{
    delete section;
}

std::shared_ptr<MitoSection> Mitochondria::appendRootSection(
    const morphio::MitoSection& section_, bool recursive)
{
    std::shared_ptr<MitoSection> ptr(new MitoSection(this, _counter, section_),
        friendDtorForSharedPtrMito);
    _register(ptr);
    _rootSections.push_back(ptr);

    if (recursive) {
        for (const auto& child : section_.children()) {
            ptr->appendSection(child, true);
        }
    }

    return ptr;
}

std::shared_ptr<MitoSection> Mitochondria::appendRootSection(
    std::shared_ptr<MitoSection> section_, bool recursive)
{
    std::shared_ptr<MitoSection> section_copy(new MitoSection(this, _counter,
                                                  *section_),
        friendDtorForSharedPtrMito);
    _register(section_copy);
    _rootSections.push_back(section_copy);

    if (recursive) {
        for (const auto& child : section_->children()) {
            section_copy->appendSection(child, true);
        }
    }

    return section_copy;
}

std::shared_ptr<MitoSection> Mitochondria::appendRootSection(
    const Property::MitochondriaPointLevel& pointProperties)
{
    std::shared_ptr<MitoSection> ptr(new MitoSection(this, _counter,
                                         pointProperties),
        friendDtorForSharedPtrMito);
    _register(ptr);
    _rootSections.push_back(ptr);

    return ptr;
}

static void _appendMitoProperties(Property::MitochondriaPointLevel& to,
    const Property::MitochondriaPointLevel& from,
    int offset = 0)
{
    _appendVector(to._sectionIds, from._sectionIds, offset);
    _appendVector(to._relativePathLengths, from._relativePathLengths,
        offset);
    _appendVector(to._diameters, from._diameters, offset);
}

const std::vector<std::shared_ptr<MitoSection>> Mitochondria::children(
    std::shared_ptr<MitoSection> section_) const
{
    try {
        return _children.at(section_->id());
    } catch (const std::out_of_range&) {
        return std::vector<std::shared_ptr<MitoSection>>();
    }
}

const std::vector<std::shared_ptr<MitoSection>>& Mitochondria::rootSections()
    const
{
    return _rootSections;
}

const std::shared_ptr<MitoSection> Mitochondria::parent(
    const std::shared_ptr<MitoSection> parent_) const
{
    return section(_parent.at(parent_->id()));
}

bool Mitochondria::isRoot(const std::shared_ptr<MitoSection> section_) const
{
    try {
        parent(section_);
        return false;
    } catch (const std::out_of_range&) {
        return true;
    }
}

const std::shared_ptr<MitoSection> Mitochondria::section(uint32_t id) const
{
    return _sections.at(id);
}

const std::map<uint32_t, std::shared_ptr<MitoSection>> Mitochondria::sections()
    const
{
    return _sections;
}

void Mitochondria::_buildMitochondria(Property::Properties& properties) const
{
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
            _appendMitoProperties(properties._mitochondriaPointLevel,
                section_->_mitoPoints);

            newIds[section_->id()] = counter++;

            for (auto child : children(section_))
                q.push(child);
        }
    }
}

const std::shared_ptr<MitoSection> Mitochondria::mitoSection(uint32_t id_) const
{
    return _sections.at(id_);
}

mito_depth_iterator Mitochondria::depth_begin(
    std::shared_ptr<MitoSection> section_) const
{
    return mito_depth_iterator(*this, section_);
}

mito_depth_iterator Mitochondria::depth_end() const
{
    return mito_depth_iterator();
}

mito_breadth_iterator Mitochondria::breadth_begin(
    std::shared_ptr<MitoSection> section_) const
{
    return mito_breadth_iterator(*this, section_);
}

mito_breadth_iterator Mitochondria::breadth_end() const
{
    return mito_breadth_iterator();
}

mito_upstream_iterator Mitochondria::upstream_begin(
    std::shared_ptr<MitoSection> section_) const
{
    return mito_upstream_iterator(*this, section_);
}

mito_upstream_iterator Mitochondria::upstream_end() const
{
    return mito_upstream_iterator();
}

uint32_t Mitochondria::_register(std::shared_ptr<MitoSection> section_)
{
    if (_sections.count(section_->id()))
        LBTHROW(SectionBuilderError("Section already exists"));
    _counter = std::max(_counter, section_->id()) + 1;

    _sections[section_->id()] = section_;
    return section_->id();
}

} // namespace mut
} // namespace morphio
