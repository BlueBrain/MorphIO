#include <cassert>
#include <iostream>
#include <fstream>

#include <morphio/mut/writers.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>

#include "../plugin/errorMessages.h"

#include <highfive/H5File.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5Object.hpp>

namespace morphio
{
namespace mut
{
namespace writer
{
void swc(const Morphology& morphology, const std::string& filename)
{
    std::ofstream myfile;
    myfile.open (filename);
    using std::setw;

    myfile << "# index" << setw(9)
           << "type" << setw(9)
           << "X" << setw(12)
           << "Y" << setw(12)
           << "Z" << setw(12)
           << "radius" << setw(13)
           << "parent\n" << std::endl;


    int segmentIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    auto soma = morphology.soma();

    if(morphology.soma()->points().size() < 1)
        throw WriterError(plugin::ErrorMessages().ERROR_WRITE_NO_SOMA());

    for (int i = 0; i < soma->points().size(); ++i){
        myfile << segmentIdOnDisk++ << setw(12) << SECTION_SOMA << setw(12)
               << soma->points()[i][0] << setw(12) << soma->points()[i][1] << setw(12)
               << soma->points()[i][2] << setw(12) << soma->diameters()[i] / 2. << setw(12)
                  << (i==0 ? -1 : segmentIdOnDisk-1) << std::endl;
    }

    for(auto it = morphology.depth_begin(); it != morphology.depth_end(); ++it) {
        int32_t sectionId = *it;
        auto section = morphology.section(sectionId);
        const auto& points = section->points();
        const auto& diameters = section->diameters();

        assert(points.size() > 0 && "Empty section");
        bool isRootSection = morphology.parent(sectionId) < 0;
        for (int i = (isRootSection ? 0 : 1); i < points.size(); ++i)
        {
            myfile << segmentIdOnDisk << setw(12) << section->type() << setw(12)
                   << points[i][0] << setw(12) << points[i][1] << setw(12)
                   << points[i][2] << setw(12) << diameters[i] / 2. << setw(12);

            if (i > (isRootSection ? 0 : 1))
                myfile << segmentIdOnDisk - 1 << std::endl;
            else {
                int32_t parentId = morphology.parent(sectionId);
                myfile << (parentId != -1 ? newIds[parentId] : 1) << std::endl;
            }

            ++segmentIdOnDisk;
        }
        newIds[section->id()] = segmentIdOnDisk - 1;
    }

    myfile.close();

}

void _write_asc_points(std::ofstream& myfile, const Points& points,
                       const std::vector<float>& diameters, int indentLevel)
{
    for (int i = 0; i < points.size(); ++i)
    {
        myfile << std::string(indentLevel, ' ') << "(" << points[i][0] << ' '
                  << points[i][1] << ' ' << points[i][2] << ' ' << diameters[i]
                  << ')' << std::endl;
    }
}

void _write_asc_section(std::ofstream& myfile, const Morphology& morpho, uint32_t id, int indentLevel)
{
    std::string indent(indentLevel, ' ');
    auto section = morpho.section(id);
    _write_asc_points(myfile, section->points(), section->diameters(), indentLevel);

    if (!morpho.children(id).empty())
    {
        auto children = morpho.children(id);
        size_t nChildren = children.size();
        for (int i = 0; i<nChildren; ++i)
        {
            myfile << indent << (i == 0 ? "(" : "|") << std::endl;
            _write_asc_section(myfile, morpho, children[i], indentLevel + 2);
        }
        myfile << indent << ")" << std::endl;
    }
}

void asc(const Morphology& morphology, const std::string& filename)
{
    std::ofstream myfile;
    myfile.open (filename);

    std::map<morphio::SectionType, std::string> header;
    header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";
    header[SECTION_APICAL_DENDRITE] = "( (Color Red)\n  (Apical)\n";

    const auto soma = morphology.soma();
    if(soma->points().size() > 0) {
        myfile << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
        _write_asc_points(myfile, soma->points(), soma->diameters(), 2);
        myfile << ")\n\n";
    } else {
        LBERROR(plugin::ErrorMessages().ERROR_WRITE_NO_SOMA());
    }


    for (auto& id : morphology.rootSections())
    {
        myfile << header.at(morphology.section(id)->type());
        _write_asc_section(myfile, morphology, id, 2);
        myfile << ")\n\n";
    }

    myfile.close();

}

template <typename T> HighFive::Attribute write_attribute(HighFive::File& file, const std::string& name, const T& version) {
    HighFive::Attribute a_version = file.createAttribute<typename T::value_type>(
        name, HighFive::DataSpace::From(version));
    a_version.write(version);
    return a_version;
}

template <typename T> HighFive::Attribute write_attribute(HighFive::Group& group, const std::string& name, const T& version) {
    HighFive::Attribute a_version = group.createAttribute<typename T::value_type>(
        name, HighFive::DataSpace::From(version));
    a_version.write(version);
    return a_version;
}

void h5(const Morphology& morpho, const std::string& filename)
{

    HighFive::File h5_file(filename, HighFive::File::ReadWrite | HighFive::File::Create |
                           HighFive::File::Truncate);



    int sectionIdOnDisk = 1;
    int i = 0;
    std::map<uint32_t, int32_t> newIds;

    std::vector<std::vector<float>> raw_points;
    std::vector<std::vector<int32_t>> raw_structure;
    std::vector<float> raw_perimeters;

    const auto &points = morpho.soma()->points();
    const auto &diameters = morpho.soma()->diameters();

    const std::size_t numberOfPoints = points.size();
    const std::size_t numberOfDiameters = diameters.size();


    if(numberOfPoints < 1)
        throw WriterError(plugin::ErrorMessages().ERROR_WRITE_NO_SOMA());
    if(numberOfPoints != numberOfDiameters)
        throw WriterError(plugin::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
                              "soma points", numberOfPoints,
                              "soma diameters", numberOfDiameters));

    bool hasPerimeterData = morpho.rootSections().size() > 0 ?
        morpho.section(morpho.rootSections()[0])->perimeters().size() > 0 :
        false;

    for(int i = 0;i<numberOfPoints; ++i) {
        raw_points.push_back({points[i][0], points[i][1], points[i][2], diameters[i]});
        if(hasPerimeterData)
            raw_perimeters.push_back(0);
    }

    raw_structure.push_back({0, SECTION_SOMA, -1});
    int offset = 0;
    offset += morpho.soma()->points().size();




    for(auto it = morpho.depth_begin(); it != morpho.depth_end(); ++it) {
        uint32_t sectionId = *it;
        uint32_t parentId = morpho.parent(sectionId);
        int parentOnDisk = (parentId != -1 ? newIds[parentId] : 0);

        auto section = morpho.section(sectionId);
        const auto &points = section->points();
        const auto &diameters = section->diameters();
        const auto &perimeters = section->perimeters();

        const std::size_t numberOfPoints = points.size();
        const std::size_t numberOfPerimeters = perimeters.size();
        raw_structure.push_back({offset, section->type(), parentOnDisk});

        for(int i = 0;i<numberOfPoints; ++i)
            raw_points.push_back({points[i][0], points[i][1], points[i][2], diameters[i]});

        if(numberOfPerimeters > 0) {

            if(numberOfPerimeters != numberOfPoints)
                throw WriterError(plugin::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
                                      "points", numberOfPoints,
                                      "perimeters", numberOfPerimeters));
            for(int i = 0;i<numberOfPerimeters; ++i)
                raw_perimeters.push_back(perimeters[i]);
        }

        newIds[section->id()] = sectionIdOnDisk++;
        offset += numberOfPoints;
    }


    HighFive::DataSet dpoints =
        h5_file.createDataSet<double>("/points", HighFive::DataSpace::From(raw_points));
    HighFive::DataSet dstructures = h5_file.createDataSet<int32_t>(
        "/structure", HighFive::DataSpace::From(raw_structure));
    HighFive::DataSet dperimeters = h5_file.createDataSet<float>(
        "/perimeters", HighFive::DataSpace::From(raw_perimeters));


    std::string METADATA = "metadata";
    HighFive::Group g_metadata = h5_file.createGroup(METADATA);

    std::vector<uint32_t> version{1,1};


    dpoints.write(raw_points);
    dstructures.write(raw_structure);
    write_attribute(g_metadata, "version", version);
    write_attribute(g_metadata, "cell_family", std::vector<uint32_t>{FAMILY_NEURON});
    write_attribute(h5_file, "comment",
                    std::vector<std::string>{" created out by morpho_tool v1"});


    if(hasPerimeterData)
        dperimeters.write(raw_perimeters);
}

} // end namespace writer
} // end namespace mut
} // end namespace morphio
