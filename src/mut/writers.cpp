#include <cassert>
#include <fstream>
#include <iostream>

#include <morphio/errorMessages.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Object.hpp>

namespace morphio {
namespace mut {
namespace writer {
template <typename T>
struct base_type
{
    using type = T;
};

/**
   A structure to get the base type of nested vectors
 **/
template <typename T>
struct base_type<std::vector<T>> : base_type<T>
{
};

void writeLine(std::ofstream& myfile, int id, int parentId, SectionType type,
    const Point& point, float diameter)
{
    using std::setw;

    myfile << std::to_string(id) << setw(12) << std::to_string(type) << " "
           << setw(12) << std::to_string(point[0]) << " " << setw(12)
           << std::to_string(point[1]) << " " << setw(12)
           << std::to_string(point[2]) << " " << setw(12)
           << std::to_string(diameter / 2.) << setw(12)
           << std::to_string(parentId) << std::endl;
}

std::string version_footnote()
{
    return std::string("Created by MorphIO v") + morphio::VERSION;
}

/**
   Only skip duplicate if it has the same diameter
 **/
bool _skipDuplicate(const std::shared_ptr<Section> section)
{
    return section->diameters()[0] == section->parent()->diameters().back();
}

void swc(const Morphology& morphology, const std::string& filename)
{
    std::ofstream myfile;
    myfile.open(filename);
    using std::setw;

    myfile << "# index" << setw(9) << "type" << setw(10) << "X" << setw(13)
           << "Y" << setw(13) << "Z" << setw(13) << "radius" << setw(13)
           << "parent" << std::endl;

    int segmentIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    auto soma = morphology.soma();

    if (!morphology.mitochondria().rootSections().empty())
        LBERROR(
            Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED,
            plugin::ErrorMessages().WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED());

    const auto& points = soma->points();
    const auto& diameters = soma->diameters();

    if (points.size() < 1)
        LBERROR(Warning::WRITE_NO_SOMA,
            plugin::ErrorMessages().WARNING_WRITE_NO_SOMA());

    for (unsigned int i = 0; i < points.size(); ++i) {
        writeLine(myfile, segmentIdOnDisk, i == 0 ? -1 : segmentIdOnDisk - 1,
            SECTION_SOMA, points[i], diameters[i]);
        ++segmentIdOnDisk;
    }

    for (auto it = morphology.depth_begin(); it != morphology.depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        const auto& points = section->points();
        const auto& diameters = section->diameters();

        assert(points.size() > 0 && "Empty section");
        bool isRootSection = section->isRoot();

        // skips duplicate point for non-root sections
        unsigned int firstPoint = ((isRootSection || !_skipDuplicate(section)) ? 0 : 1);
        for (unsigned int i = firstPoint; i < points.size(); ++i) {
            int parentIdOnDisk;
            if (i > firstPoint)
                parentIdOnDisk = segmentIdOnDisk - 1;
            else {
                parentIdOnDisk = (isRootSection ? (soma->points().empty() ? -1 : 1)
                                                : newIds[section->parent()->id()]);
            }

            writeLine(myfile, segmentIdOnDisk, parentIdOnDisk, section->type(),
                points[i], diameters[i]);

            ++segmentIdOnDisk;
        }
        newIds[section->id()] = segmentIdOnDisk - 1;
    }

    myfile << "\n# " << version_footnote() << std::endl;
    myfile.close();
}

void _write_asc_points(std::ofstream& myfile, const Points& points,
    const std::vector<float>& diameters, int indentLevel)
{
    for (unsigned int i = 0; i < points.size(); ++i) {
        myfile << std::string(indentLevel, ' ') << "("
               << std::to_string(points[i][0]) << ' '
               << std::to_string(points[i][1]) << ' '
               << std::to_string(points[i][2]) << ' '
               << std::to_string(diameters[i]) << ')' << std::endl;
    }
}

void _write_asc_section(std::ofstream& myfile, const Morphology& morpho,
    const std::shared_ptr<Section>& section,
    int indentLevel)
{
    std::string indent(indentLevel, ' ');
    _write_asc_points(myfile, section->points(), section->diameters(),
        indentLevel);

    if (!section->children().empty()) {
        auto children = section->children();
        size_t nChildren = children.size();
        for (unsigned int i = 0; i < nChildren; ++i) {
            myfile << indent << (i == 0 ? "(" : "|") << std::endl;
            _write_asc_section(myfile, morpho, children[i], indentLevel + 2);
        }
        myfile << indent << ")" << std::endl;
    }
}

void asc(const Morphology& morphology, const std::string& filename)
{
    std::ofstream myfile;
    myfile.open(filename);

    if (!morphology.mitochondria().rootSections().empty())
        LBERROR(
            Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED,
            plugin::ErrorMessages().WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED());

    std::map<morphio::SectionType, std::string> header;
    header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";
    header[SECTION_APICAL_DENDRITE] = "( (Color Red)\n  (Apical)\n";

    const auto soma = morphology.soma();
    if (soma->points().size() > 0) {
        myfile << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
        _write_asc_points(myfile, soma->points(), soma->diameters(), 2);
        myfile << ")\n\n";
    } else {
        LBERROR(Warning::WRITE_NO_SOMA,
            plugin::ErrorMessages().WARNING_WRITE_NO_SOMA());
    }

    for (auto& section : morphology.rootSections()) {
        myfile << header.at(section->type());
        _write_asc_section(myfile, morphology, section, 2);
        myfile << ")\n\n";
    }

    myfile << "; " << version_footnote() << std::endl;
    myfile.close();
}

template <typename T>
HighFive::Attribute write_attribute(HighFive::File& file,
    const std::string& name, const T& version)
{
    HighFive::Attribute a_version = file.createAttribute<typename T::value_type>(name,
        HighFive::DataSpace::From(
            version));
    a_version.write(version);
    return a_version;
}

template <typename T>
HighFive::Attribute write_attribute(HighFive::Group& group,
    const std::string& name, const T& version)
{
    HighFive::Attribute a_version = group.createAttribute<typename T::value_type>(name,
        HighFive::DataSpace::From(
            version));
    a_version.write(version);
    return a_version;
}

template <typename T>
void write_dataset(HighFive::File& file, const std::string& name, const T& raw)
{
    HighFive::DataSet dpoints = file.createDataSet<typename base_type<T>::type>(
        name, HighFive::DataSpace::From(raw));

    dpoints.write(raw);
}

template <typename T>
void write_dataset(HighFive::Group& file, const std::string& name, const T& raw)
{
    HighFive::DataSet dpoints = file.createDataSet<typename base_type<T>::type>(
        name, HighFive::DataSpace::From(raw));

    dpoints.write(raw);
}

void mitochondriaH5(HighFive::File& h5_file, const Mitochondria& mitochondria)
{
    if (mitochondria.rootSections().empty())
        return;

    Property::Properties properties;
    mitochondria._buildMitochondria(properties);
    auto& p = properties._mitochondriaPointLevel;
    size_t size = p._diameters.size();

    std::vector<std::vector<float>> points;
    std::vector<std::vector<int32_t>> structure;
    for (unsigned int i = 0; i < size; ++i) {
        points.push_back({(float)p._sectionIds[i], p._relativePathLengths[i],
            p._diameters[i]});
    }

    auto& s = properties._mitochondriaSectionLevel;
    for (unsigned int i = 0; i < s._sections.size(); ++i) {
        structure.push_back({s._sections[i][0], s._sections[i][1]});
    }

    HighFive::Group g_mitochondria = h5_file.createGroup("mitochondria");

    write_dataset(g_mitochondria, "/mitochondria/points", points);
    write_dataset(h5_file, "/mitochondria/structure", structure);
}

void h5(const Morphology& morpho, const std::string& filename)
{
    HighFive::File h5_file(filename, HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);

    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;

    std::vector<std::vector<float>> raw_points;
    std::vector<std::vector<int32_t>> raw_structure;
    std::vector<float> raw_perimeters;

    const auto& points = morpho.soma()->points();
    const auto& diameters = morpho.soma()->diameters();

    const std::size_t numberOfPoints = points.size();
    const std::size_t numberOfDiameters = diameters.size();

    if (numberOfPoints < 1)
        LBERROR(Warning::WRITE_NO_SOMA,
            plugin::ErrorMessages().WARNING_WRITE_NO_SOMA());
    if (numberOfPoints != numberOfDiameters)
        throw WriterError(plugin::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
            "soma points", numberOfPoints, "soma diameters",
            numberOfDiameters));

    bool hasPerimeterData = morpho.rootSections().size() > 0
                                ? morpho.rootSections()[0]->perimeters().size() > 0
                                : false;

    for (unsigned int i = 0; i < numberOfPoints; ++i) {
        raw_points.push_back(
            {points[i][0], points[i][1], points[i][2], diameters[i]});

        // If the morphology has some perimeter data, we need to fill some
        // perimeter dummy value in the soma range of the data structure to keep
        // the length matching
        if (hasPerimeterData)
            raw_perimeters.push_back(0);
    }

    raw_structure.push_back({0, SECTION_SOMA, -1});
    int offset = 0;
    offset += morpho.soma()->points().size();

    for (auto it = morpho.depth_begin(); it != morpho.depth_end(); ++it) {
        std::shared_ptr<Section> section = *it;
        int parentOnDisk = (section->isRoot() ? 0 : newIds[section->parent()->id()]);

        const auto& points = section->points();
        const auto& diameters = section->diameters();
        const auto& perimeters = section->perimeters();

        const std::size_t numberOfPoints = points.size();
        const std::size_t numberOfPerimeters = perimeters.size();
        raw_structure.push_back({offset, section->type(), parentOnDisk});

        for (unsigned int i = 0; i < numberOfPoints; ++i)
            raw_points.push_back(
                {points[i][0], points[i][1], points[i][2], diameters[i]});

        if (numberOfPerimeters > 0) {
            if (numberOfPerimeters != numberOfPoints)
                throw WriterError(
                    plugin::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
                        "points", numberOfPoints, "perimeters",
                        numberOfPerimeters));
            for (unsigned int i = 0; i < numberOfPerimeters; ++i)
                raw_perimeters.push_back(perimeters[i]);
        }

        newIds[section->id()] = sectionIdOnDisk++;
        offset += numberOfPoints;
    }

    write_dataset(h5_file, "/points", raw_points);
    write_dataset(h5_file, "/structure", raw_structure);

    HighFive::Group g_metadata = h5_file.createGroup("metadata");

    write_attribute(g_metadata, "version", std::vector<uint32_t>{1, 1});
    write_attribute(g_metadata, "cell_family",
        std::vector<uint32_t>{FAMILY_NEURON});
    write_attribute(h5_file, "comment",
        std::vector<std::string>{version_footnote()});

    if (hasPerimeterData)
        write_dataset(h5_file, "/perimeters", raw_perimeters);

    mitochondriaH5(h5_file, morpho.mitochondria());
}

} // end namespace writer
} // end namespace mut
} // end namespace morphio
