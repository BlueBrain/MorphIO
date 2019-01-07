#include <cassert>
#include <iostream>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/morphology.h>

#include "plugin/morphologyASC.h"
#include "plugin/morphologyHDF5.h"
#include "plugin/morphologySWC.h"

namespace morphio {
void buildChildren(std::shared_ptr<Property::Properties> properties);
SomaType getSomaType(uint32_t nSomaPoints);

Morphology::Morphology(const URI& source, unsigned int options)
{
    const size_t pos = source.find_last_of(".");
    if (pos == std::string::npos)
        LBTHROW(UnknownFileType("File has no extension"));

    if (access(source.c_str(), F_OK) == -1)
        LBTHROW(RawDataError("File: " + source + " does not exist."));

    std::string extension;

    for (auto& c : source.substr(pos))
        extension += std::tolower(c);

    auto loader = [&source, &options, &extension]() {
        if (extension == ".h5")
            return plugin::h5::load(source);
        if (extension == ".asc")
            return plugin::asc::load(source, options);
        if (extension == ".swc")
            return plugin::swc::load(source, options);
        LBTHROW(UnknownFileType(
            "Unhandled file type: only SWC, ASC and H5 are supported"));
    };

    _properties = std::make_shared<Property::Properties>(loader());

    buildChildren(_properties);

    if (version() != MORPHOLOGY_VERSION_SWC_1)
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());

    // Sad trick because, contrary to SWC and ASC, H5 does not create a
    // mut::Morphology object on which we can directly call
    // mut::Morphology::applyModifiers
    if (options && (version() == MORPHOLOGY_VERSION_H5_1 || version() == MORPHOLOGY_VERSION_H5_1_1 || version() == MORPHOLOGY_VERSION_H5_2)) {
        mut::Morphology mutable_morph(*this);
        mutable_morph.sanitize();
        mutable_morph.applyModifiers(options);
        _properties = std::make_shared<Property::Properties>(
            mutable_morph.buildReadOnly());
        buildChildren(_properties);
    }
}

Morphology::Morphology(mut::Morphology morphology)
{
    morphology.sanitize();
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology() {}

bool Morphology::operator==(const Morphology& other) const
{
    if (this->_properties == other._properties)
        return true;

    // constexpr float epsilon = 1e-5;
    // std::array<float, 2> soma_surfaces{this->soma().surface(),
    //                                    other.soma().surface()};
    // std::cout << "std::abs(soma_surfaces[1] - soma_surfaces[0]): " <<
    // std::to_string(std::abs(soma_surfaces[1] - soma_surfaces[0])) <<
    // std::endl; if(std::abs(soma_surfaces[1] - soma_surfaces[0]) > epsilon) {
    //     LBERROR("Soma surfaces differs: " + std::to_string(soma_surfaces[0])
    //     +
    //             " VS " + std::to_string(soma_surfaces[1]));
    //     return false;
    // }

    return (this->_properties && other._properties && *(this->_properties) == *(other._properties));
}

bool Morphology::operator!=(const Morphology& other) const
{
    return !this->operator==(other);
}

const Soma Morphology::soma() const
{
    return Soma(_properties);
}

const Mitochondria Morphology::mitochondria() const
{
    return Mitochondria(_properties);
}

const std::vector<Property::Annotation> Morphology::annotations() const
{
    return _properties->_annotations;
}

const Section Morphology::section(const uint32_t& id) const
{
    return Section(id, _properties);
}

const std::vector<Section> Morphology::rootSections() const
{
    std::vector<Section> result;
    try {
        const std::vector<uint32_t>& children = _properties->children<morphio::Property::Section>().at(-1);
        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }

        return result;
    } catch (const std::out_of_range& oor) {
        return result;
    }
}

const std::vector<Section> Morphology::sections() const
{
    // TODO: Make this more performant when needed
    std::vector<Section> sections;
    for (uint i = 1; i < _properties->get<morphio::Property::Section>().size();
         ++i) {
        sections.push_back(section(i));
    }
    return sections;
}

template <typename Property>
const std::vector<typename Property::Type>& Morphology::get() const
{
    return _properties->get<Property>();
}

const Points& Morphology::points() const
{
    return get<Property::Point>();
}
const std::vector<float>& Morphology::diameters() const
{
    return get<Property::Diameter>();
}
const std::vector<float>& Morphology::perimeters() const
{
    return get<Property::Perimeter>();
}
const std::vector<SectionType>& Morphology::sectionTypes() const
{
    return get<Property::SectionType>();
}

const CellFamily& Morphology::cellFamily() const
{
    return _properties->cellFamily();
}

const SomaType& Morphology::somaType() const
{
    return _properties->somaType();
}

const MorphologyVersion& Morphology::version() const
{
    return _properties->version();
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

SomaType getSomaType(uint32_t nSomaPoints)
{
    try {
        return std::map<uint32_t, SomaType>{{0, SOMA_UNDEFINED},
            {1, SOMA_SINGLE_POINT},
            {2, SOMA_UNDEFINED}}
            .at(nSomaPoints);
    } catch (const std::out_of_range& oor) {
        return SOMA_SIMPLE_CONTOUR;
    }
}

void buildChildren(std::shared_ptr<Property::Properties> properties)
{
    {
        const auto& sections = properties->get<Property::Section>();
        auto& children = properties->_sectionLevel._children;

        for (size_t i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }

    {
        const auto& sections = properties->get<Property::MitoSection>();
        auto& children = properties->_mitochondriaSectionLevel._children;

        for (size_t i = 0; i < sections.size(); ++i) {
            const int32_t parent = sections[i][1];
            children[parent].push_back(i);
        }
    }
}

} // namespace morphio
