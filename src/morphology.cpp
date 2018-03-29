#include <cassert>
#include <iostream>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/morphology.h>

#include "plugin/morphologyASC.h"
#include "plugin/morphologyHDF5.h"
#include "plugin/morphologySWC.h"

namespace morphio
{

enum SomaClasses {
    SOMA_CONTOUR,
    SOMA_CYLINDER
};

std::map<MorphologyVersion, SomaClasses> _SOMA_CONFIG{
    // after much debate (https://github.com/BlueBrain/NeuroM/issues/597)
    // and research:
    //
    //  Cannon et al., 1998: http://www.sciencedirect.com/science/article/pii/S0165027098000910
    //    'Each line has the same seven fields: numbered point index, user defined flag denoting the
    //    specific part of the structure (cell body, dendrite, axon etc.), three-dimensional position
    //    (x, y and z, in mm), radius (r, in mm), and the parent point index'
    //
    //  Ascoli et al., 2001: http://www.jstor.org/stable/3067144
    //    'In the SWC format, dendritic segments are characterized by an identification number, a
    //    type (to distinguish basal, apical, proximal, distal and lateral trees), the x, y, z
    //    positions of the cylinder ending point (in pm with respect to a fixed reference point), a
    //    radius value (also in pm), and the identification number of the 'parent', i.e. the adjacent
    //    cylinder in the path to the soma (the parent of the root being the soma itself)."
    //
    // that the SWC format uses cylinders to represent the soma.
};

SomaType getSomaType(uint32_t nSomaPoints) {
    try {
        return std::map<uint32_t, SomaType>{
            {0, SOMA_UNDEFINED},
            {1, SOMA_SINGLE_POINT},
            {3, SOMA_THREE_POINTS},
            {2, SOMA_UNDEFINED}}.at(nSomaPoints);
    }
    catch (const std::out_of_range& oor)
    {
        return SOMA_SIMPLE_CONTOUR;
    }
}

void buildChildren(std::shared_ptr<Property::Properties> properties)
{
    const auto& sections = properties->get<Property::Section>();
    auto& children = properties->_sectionLevel._children;

    for (size_t i = 0; i < sections.size(); ++i)
    {
        const int32_t parent = sections[i][1];
        if (parent != -1)
            children[parent].push_back(i);
    }
}



Morphology::Morphology(const URI& source, unsigned int options)
{
    const size_t pos = source.find_last_of(".");
    assert(pos != std::string::npos);
    if (access(source.c_str(), F_OK) == -1)
        LBTHROW(RawDataError("File: " + source + " does not exist."));

    std::string extension;

    for(auto& c : source.substr(pos))
        extension += std::tolower(c);

    if (extension == ".h5")
    {
        _properties =
            std::make_shared<Property::Properties>(plugin::h5::load(source, options));
    }
    else if (extension == ".swc")
    {
        _properties =
            std::make_shared<Property::Properties>(plugin::swc::load(source, options));
    }
    else if (extension == ".asc")
    {
        _properties =
            std::make_shared<Property::Properties>(plugin::asc::load(source, options));
    }
    else
    {
        LBTHROW(UnknownFileType("unhandled file type"));
    }

    buildChildren(_properties);

    if(version() != MORPHOLOGY_VERSION_SWC_1)
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());

    if(options &&
       version() == MORPHOLOGY_VERSION_H5_1 ||
       version() == MORPHOLOGY_VERSION_H5_1_1 ||
       version() == MORPHOLOGY_VERSION_H5_2) {
        mut::Morphology mutable_morph(*this);
        mutable_morph.applyModifiers(options);
        _properties = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
    }
}

Morphology::Morphology(const mut::Morphology& morphology)
{
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

Morphology::Morphology(Morphology&&) = default;
Morphology& Morphology::operator=(Morphology&&) = default;

Morphology::~Morphology()
{
}

bool Morphology::operator==(const Morphology& other) const {
    return this->_properties == other._properties ||
        (this->_properties && other._properties &&
         *(this->_properties) == *(other._properties));
}


const Soma Morphology::soma() const
{
    return Soma(_properties);
}

const Section Morphology::section(const uint32_t& id) const
{
    return Section(id, _properties);
}

const std::vector<Section> Morphology::rootSections() const
{
    std::vector<Section> result;
    try
    {
        const std::vector<uint32_t>& children = _properties->children().at(0);
        result.reserve(children.size());
        for (auto id: children)
            result.push_back(section(id));
        return result;
    }
    catch (const std::out_of_range& oor)
    {
        return result;
    }
}

const std::vector<Section> Morphology::sections() const
{
    std::vector<Section> sections;
    for (int i = 0; i < _properties->get<morphio::Property::Section>().size();
         ++i)
    {
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

} // namespace morphio
