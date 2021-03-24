#include <fstream>
#include <memory>
#include <streambuf>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/morphology.h>

#include "readers/morphologyASC.h"
#include "readers/morphologyHDF5.h"
#include "readers/morphologySWC.h"

namespace morphio {
void buildChildren(std::shared_ptr<Property::Properties> properties);
SomaType getSomaType(long unsigned int nSomaPoints);
Property::Properties loadURI(const std::string& source, unsigned int options);

Morphology::Morphology(const Property::Properties& properties, unsigned int options)
    : _properties(std::make_shared<Property::Properties>(properties)) {
    buildChildren(_properties);

    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());

    // For SWC and ASC, sanitization and modifier application are already taken care of by
    // their respective loaders
    if (properties._cellLevel.fileFormat() == "h5") {
        mut::Morphology mutable_morph(*this);
        if (options) {
            mutable_morph.applyModifiers(options);
        }
        _properties = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
        buildChildren(_properties);
    }
}

Morphology::Morphology(const HighFive::Group& group, unsigned int options)
    : Morphology(readers::h5::load(group), options) {}

Morphology::Morphology(const std::string& source, unsigned int options)
    : Morphology(loadURI(source, options), options) {}

Morphology::Morphology(mut::Morphology morphology) {
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

Morphology::Morphology(Morphology&&) noexcept = default;
Morphology& Morphology::operator=(Morphology&&) noexcept = default;

Morphology::~Morphology() = default;

Soma Morphology::soma() const {
    return Soma(_properties);
}

Mitochondria Morphology::mitochondria() const {
    return Mitochondria(_properties);
}

const EndoplasmicReticulum Morphology::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

const std::vector<Property::Annotation>& Morphology::annotations() const {
    return _properties->_cellLevel._annotations;
}

const std::vector<Property::Marker>& Morphology::markers() const {
    return _properties->_cellLevel._markers;
}

Section Morphology::section(uint32_t id) const {
    return {id, _properties};
}

std::vector<Section> Morphology::rootSections() const {
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& children =
            _properties->children<morphio::Property::Section>().at(-1);
        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }

        return result;
    } catch (const std::out_of_range&) {
        return result;
    }
}

std::vector<Section> Morphology::sections() const {
    // TODO: Make this more performant when needed
    std::vector<Section> sections_;
    auto count = _properties->get<morphio::Property::Section>().size();
    sections_.reserve(count);
    for (unsigned int i = 0; i < count; ++i) {
        sections_.emplace_back(section(i));
    }
    return sections_;
}

template <typename Property>
const std::vector<typename Property::Type>& Morphology::get() const {
    return _properties->get<Property>();
}

const Points& Morphology::points() const noexcept {
    return get<Property::Point>();
}

std::vector<uint32_t> Morphology::sectionOffsets() const {
    const std::vector<Property::Section::Type>& indices_and_parents = get<Property::Section>();
    auto size = indices_and_parents.size();
    std::vector<uint32_t> indices(size + 1);
    std::transform(indices_and_parents.begin(),
                   indices_and_parents.end(),
                   indices.begin(),
                   [](const Property::Section::Type& pair) { return pair[0]; });
    indices[size] = static_cast<uint32_t>(points().size());
    return indices;
}

const std::vector<morphio::floatType>& Morphology::diameters() const {
    return get<Property::Diameter>();
}

const std::vector<morphio::floatType>& Morphology::perimeters() const {
    return get<Property::Perimeter>();
}

const std::vector<SectionType>& Morphology::sectionTypes() const {
    return get<Property::SectionType>();
}

const CellFamily& Morphology::cellFamily() const {
    return _properties->cellFamily();
}

const SomaType& Morphology::somaType() const {
    return _properties->somaType();
}

const std::map<int, std::vector<unsigned int>>& Morphology::connectivity() const {
    return _properties->children<Property::Section>();
}

const MorphologyVersion& Morphology::version() const {
    return _properties->version();
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

SomaType getSomaType(long unsigned int nSomaPoints) {
    try {
        return std::map<long unsigned int, SomaType>{{0, SOMA_UNDEFINED},
                                                     {1, SOMA_SINGLE_POINT},
                                                     {2, SOMA_UNDEFINED}}
            .at(nSomaPoints);
    } catch (const std::out_of_range&) {
        return SOMA_SIMPLE_CONTOUR;
    }
}

void buildChildren(std::shared_ptr<Property::Properties> properties) {
    {
        const auto& sections = properties->get<Property::Section>();
        auto& children = properties->_sectionLevel._children;

        for (unsigned int i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }

    {
        const auto& sections = properties->get<Property::MitoSection>();
        auto& children = properties->_mitochondriaSectionLevel._children;

        for (unsigned int i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }
}

Property::Properties loadURI(const std::string& source, unsigned int options) {
    const size_t pos = source.find_last_of(".");
    if (pos == std::string::npos)
        throw(UnknownFileType("File has no extension"));

    // Cross-platform check of file existance
    std::ifstream file(source.c_str());
    if (!file) {
        throw(RawDataError("File: " + source + " does not exist."));
    }

    std::string extension = source.substr(pos);

    auto loader = [&source, &options, &extension]() {
        if (extension == ".h5" || extension == ".H5")
            return readers::h5::load(source);
        if (extension == ".asc" || extension == ".ASC")
            return readers::asc::load(source, options);
        if (extension == ".swc" || extension == ".SWC")
            return readers::swc::load(source, options);
        throw(UnknownFileType("Unhandled file type: only SWC, ASC and H5 are supported"));
    };

    return loader();
}

}  // namespace morphio
