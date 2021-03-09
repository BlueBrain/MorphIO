#include <cassert>
#include <fstream>

#include <morphio/errorMessages.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/version.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Object.hpp>

namespace {

/**
   A structure to get the base type of nested vectors
   https://stackoverflow.com/a/30960730
 **/
template <typename T>
struct base_type {
    using type = T;
};

template <typename T>
struct base_type<std::vector<T>>: base_type<T> {};

constexpr int FLOAT_PRECISION_PRINT = 9;

bool hasPerimeterData(const morphio::mut::Morphology& morpho) {
    return !morpho.rootSections().empty() && !morpho.rootSections().front()->perimeters().empty();
}

void writeLine(std::ofstream& myfile,
               int id,
               int parentId,
               morphio::SectionType type,
               const morphio::Point& point,
               morphio::floatType diameter) {
    using std::setw;


    myfile << std::to_string(id) << setw(12) << std::to_string(type) << ' ' << setw(12);
    myfile << std::fixed
#if !defined(MORPHIO_USE_DOUBLE)
           << std::setprecision(FLOAT_PRECISION_PRINT)
#endif
           << point[0] << ' ' << setw(12) << point[1] << ' ' << setw(12) << point[2] << ' '
           << setw(12) << diameter / 2 << setw(12);
    myfile << std::to_string(parentId) << '\n';
}

std::string version_string() {
    return std::string("Created by MorphIO v") + morphio::getVersionString();
}

/**
   Only skip duplicate if it has the same diameter
 **/
bool _skipDuplicate(const std::shared_ptr<morphio::mut::Section>& section) {
    return section->diameters().front() == section->parent()->diameters().back();
}

}  // anonymous namespace

namespace morphio {
namespace mut {
namespace writer {

void swc(const Morphology& morphology, const std::string& filename) {
    const auto& soma = morphology.soma();
    const auto& soma_points = soma->points();
    if (soma_points.empty() && morphology.rootSections().empty()) {
        printError(Warning::WRITE_EMPTY_MORPHOLOGY,
                   readers::ErrorMessages().WARNING_WRITE_EMPTY_MORPHOLOGY());
        return;
    }

    if (hasPerimeterData(morphology)) {
        throw WriterError(readers::ErrorMessages().ERROR_PERIMETER_DATA_NOT_WRITABLE());
    }

    std::ofstream myfile(filename);
    using std::setw;

    myfile << "# " << version_string() << std::endl;
    myfile << "# index" << setw(9) << "type" << setw(10) << 'X' << setw(13) << 'Y' << setw(13)
           << 'Z' << setw(13) << "radius" << setw(13) << "parent" << std::endl;

    int segmentIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;

    if (!morphology.mitochondria().rootSections().empty())
        printError(Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED,
                   readers::ErrorMessages().WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED());

    const auto& soma_diameters = soma->diameters();

    if (soma_points.empty())
        printError(Warning::WRITE_NO_SOMA, readers::ErrorMessages().WARNING_WRITE_NO_SOMA());

    for (unsigned int i = 0; i < soma_points.size(); ++i) {
        writeLine(myfile,
                  segmentIdOnDisk,
                  i == 0 ? -1 : segmentIdOnDisk - 1,
                  SECTION_SOMA,
                  soma_points[i],
                  soma_diameters[i]);
        ++segmentIdOnDisk;
    }

    for (auto it = morphology.depth_begin(); it != morphology.depth_end(); ++it) {
        const std::shared_ptr<Section>& section = *it;
        const auto& points = section->points();
        const auto& diameters = section->diameters();

        assert(!points.empty() && "Empty section");
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

            writeLine(
                myfile, segmentIdOnDisk, parentIdOnDisk, section->type(), points[i], diameters[i]);

            ++segmentIdOnDisk;
        }
        newIds[section->id()] = segmentIdOnDisk - 1;
    }
}

static void _write_asc_points(std::ofstream& myfile,
                              const Points& points,
                              const std::vector<morphio::floatType>& diameters,
                              size_t indentLevel) {
    for (unsigned int i = 0; i < points.size(); ++i) {
        myfile << std::fixed << std::setprecision(FLOAT_PRECISION_PRINT)
               << std::string(indentLevel, ' ') << '(' << points[i][0] << ' ' << points[i][1] << ' '
               << points[i][2] << ' ' << diameters[i] << ")\n";
    }
}

static void _write_asc_section(std::ofstream& myfile,
                               const Morphology& morpho,
                               const std::shared_ptr<Section>& section,
                               size_t indentLevel) {
    std::string indent(indentLevel, ' ');
    _write_asc_points(myfile, section->points(), section->diameters(), indentLevel);

    if (!section->children().empty()) {
        auto children = section->children();
        size_t nChildren = children.size();
        for (unsigned int i = 0; i < nChildren; ++i) {
            myfile << indent << (i == 0 ? "(\n" : "|\n");
            _write_asc_section(myfile, morpho, children[i], indentLevel + 2);
        }
        myfile << indent << ")\n";
    }
}

void asc(const Morphology& morphology, const std::string& filename) {
    const auto& soma = morphology.soma();
    if (soma->points().empty() && morphology.rootSections().empty()) {
        printError(Warning::WRITE_EMPTY_MORPHOLOGY,
                   readers::ErrorMessages().WARNING_WRITE_EMPTY_MORPHOLOGY());
        return;
    }

    if (hasPerimeterData(morphology)) {
        throw WriterError(readers::ErrorMessages().ERROR_PERIMETER_DATA_NOT_WRITABLE());
    }

    std::ofstream myfile(filename);

    if (!morphology.mitochondria().rootSections().empty())
        printError(Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED,
                   readers::ErrorMessages().WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED());

    std::map<morphio::SectionType, std::string> header;
    header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";
    header[SECTION_APICAL_DENDRITE] = "( (Color Red)\n  (Apical)\n";

    if (!soma->points().empty()) {
        myfile << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
        _write_asc_points(myfile, soma->points(), soma->diameters(), 2);
        myfile << ")\n\n";
    } else {
        printError(Warning::WRITE_NO_SOMA, readers::ErrorMessages().WARNING_WRITE_NO_SOMA());
    }

    for (const auto& section : morphology.rootSections()) {
        myfile << header.at(section->type());
        _write_asc_section(myfile, morphology, section, 2);
        myfile << ")\n\n";
    }

    myfile << "; " << version_string() << '\n';
}

template <typename T>
HighFive::Attribute write_attribute(HighFive::File& file,
                                    const std::string& name,
                                    const T& version) {
    HighFive::Attribute a_version =
        file.createAttribute<typename T::value_type>(name, HighFive::DataSpace::From(version));
    a_version.write(version);
    return a_version;
}

template <typename T>
HighFive::Attribute write_attribute(HighFive::Group& group,
                                    const std::string& name,
                                    const T& version) {
    HighFive::Attribute a_version =
        group.createAttribute<typename T::value_type>(name, HighFive::DataSpace::From(version));
    a_version.write(version);
    return a_version;
}

template <typename T>
void write_dataset(HighFive::File& file, const std::string& name, const T& raw) {
    HighFive::DataSet dpoints =
        file.createDataSet<typename base_type<T>::type>(name, HighFive::DataSpace::From(raw));

    dpoints.write(raw);
}

template <typename T>
void write_dataset(HighFive::Group& file, const std::string& name, const T& raw) {
    HighFive::DataSet dpoints =
        file.createDataSet<typename base_type<T>::type>(name, HighFive::DataSpace::From(raw));

    dpoints.write(raw);
}

static void mitochondriaH5(HighFive::File& h5_file, const Mitochondria& mitochondria) {
    if (mitochondria.rootSections().empty())
        return;

    Property::Properties properties;
    mitochondria._buildMitochondria(properties);
    auto& p = properties._mitochondriaPointLevel;
    size_t size = p._diameters.size();

    std::vector<std::vector<morphio::floatType>> points;
    std::vector<std::vector<int32_t>> structure;
    points.reserve(size);
    for (unsigned int i = 0; i < size; ++i) {
        points.push_back({static_cast<morphio::floatType>(p._sectionIds[i]),
                          p._relativePathLengths[i],
                          p._diameters[i]});
    }

    auto& s = properties._mitochondriaSectionLevel;
    structure.reserve(s._sections.size());
    for (const auto& section : s._sections) {
        structure.push_back({section[0], section[1]});
    }

    HighFive::Group g_organelles = h5_file.createGroup("organelles");
    HighFive::Group g_mitochondria = g_organelles.createGroup("mitochondria");

    write_dataset(g_mitochondria, "points", points);
    write_dataset(g_mitochondria, "structure", structure);
}


static void endoplasmicReticulumH5(HighFive::File& h5_file, const EndoplasmicReticulum& reticulum) {
    if (reticulum.sectionIndices().empty())
        return;

    HighFive::Group g_organelles = h5_file.createGroup("organelles");
    HighFive::Group g_reticulum = g_organelles.createGroup("endoplasmic_reticulum");

    write_dataset(g_reticulum, "section_index", reticulum.sectionIndices());
    write_dataset(g_reticulum, "volume", reticulum.volumes());
    write_dataset(g_reticulum, "filament_count", reticulum.filamentCounts());
    write_dataset(g_reticulum, "surface_area", reticulum.surfaceAreas());
}


void h5(const Morphology& morpho, const std::string& filename) {
    const auto& somaPoints = morpho.soma()->points();
    const auto numberOfSomaPoints = somaPoints.size();

    if (numberOfSomaPoints < 1) {
        if (morpho.rootSections().empty()) {
            printError(Warning::WRITE_EMPTY_MORPHOLOGY,
                       readers::ErrorMessages().WARNING_WRITE_EMPTY_MORPHOLOGY());
            return;
        }
        printError(Warning::WRITE_NO_SOMA, readers::ErrorMessages().WARNING_WRITE_NO_SOMA());
    }


    HighFive::File h5_file(filename,
                           HighFive::File::ReadWrite | HighFive::File::Create |
                               HighFive::File::Truncate);

    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;

    std::vector<std::vector<morphio::floatType>> raw_points;
    std::vector<std::vector<int32_t>> raw_structure;
    std::vector<morphio::floatType> raw_perimeters;

    const auto& somaDiameters = morpho.soma()->diameters();

    const auto numberOfSomaDiameters = somaDiameters.size();


    if (numberOfSomaPoints != numberOfSomaDiameters)
        throw WriterError(readers::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
            "soma points", numberOfSomaPoints, "soma diameters", numberOfSomaDiameters));


    bool hasPerimeterData_ = hasPerimeterData(morpho);

    for (unsigned int i = 0; i < numberOfSomaPoints; ++i) {
        raw_points.push_back(
            {somaPoints[i][0], somaPoints[i][1], somaPoints[i][2], somaDiameters[i]});

        // If the morphology has some perimeter data, we need to fill some
        // perimeter dummy value in the soma range of the data structure to keep
        // the length matching
        if (hasPerimeterData_) {
            raw_perimeters.push_back(0);
        }
    }

    raw_structure.push_back({0, SECTION_SOMA, -1});
    size_t offset = 0;
    offset += morpho.soma()->points().size();

    for (auto it = morpho.depth_begin(); it != morpho.depth_end(); ++it) {
        const std::shared_ptr<Section>& section = *it;
        int parentOnDisk = (section->isRoot() ? 0 : newIds[section->parent()->id()]);

        const auto& points = section->points();
        const auto& diameters = section->diameters();
        const auto& perimeters = section->perimeters();

        const auto numberOfPoints = points.size();
        const auto numberOfPerimeters = perimeters.size();
        raw_structure.push_back({static_cast<int>(offset), section->type(), parentOnDisk});

        for (unsigned int i = 0; i < numberOfPoints; ++i)
            raw_points.push_back({points[i][0], points[i][1], points[i][2], diameters[i]});

        if (numberOfPerimeters > 0) {
            if (numberOfPerimeters != numberOfPoints)
                throw WriterError(readers::ErrorMessages().ERROR_VECTOR_LENGTH_MISMATCH(
                    "points", numberOfPoints, "perimeters", numberOfPerimeters));
            for (unsigned int i = 0; i < numberOfPerimeters; ++i)
                raw_perimeters.push_back(perimeters[i]);
        }

        newIds[section->id()] = sectionIdOnDisk++;
        offset += numberOfPoints;
    }

    write_dataset(h5_file, "/points", raw_points);
    write_dataset(h5_file, "/structure", raw_structure);

    HighFive::Group g_metadata = h5_file.createGroup("metadata");

    write_attribute(g_metadata, "version", std::vector<uint32_t>{1, 2});
    write_attribute(g_metadata,
                    "cell_family",
                    std::vector<uint32_t>{static_cast<uint32_t>(morpho.cellFamily())});
    write_attribute(h5_file, "comment", std::vector<std::string>{version_string()});

    if (hasPerimeterData_) {
        write_dataset(h5_file, "/perimeters", raw_perimeters);
    }

    mitochondriaH5(h5_file, morpho.mitochondria());
    endoplasmicReticulumH5(h5_file, morpho.endoplasmicReticulum());
}

}  // end namespace writer
}  // end namespace mut
}  // end namespace morphio
