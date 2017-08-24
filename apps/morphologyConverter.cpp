
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include <brion/brion.h>
#include <brion/detail/morphologyHDF5.h>
#include <brion/detail/silenceHDF5.h>
#include <brion/detail/utilsHDF5.h>

#include <lunchbox/clock.h>
#include <lunchbox/file.h>
#include <lunchbox/log.h>
#include <lunchbox/term.h>

#include <H5Cpp.h>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using boost::lexical_cast;
void writeMorphology(const brion::Morphology& in, const std::string& output);

int main(int argc, char* argv[])
{
    po::options_description options(lunchbox::getFilename(argv[0]),
                                    lunchbox::term::getSize().first);

    // clang-format off
    options.add_options()
        ( "help,h", "Produce help message" )
        ( "version,v", "Show program name/version banner and exit" )
        ( "input,i", po::value< std::string >(), "Input morphology" )
        ( "output,o", po::value< std::string >(), "Output H5 V1.1 morphology" )
    ;
    // clang-format on
    po::variables_map vm;

    try
    {
        po::store(po::parse_command_line(argc, argv, options), vm);
        po::notify(vm);
    }
    catch (...)
    {
        std::cout << "Unknown arguments provided\n\n";
        std::cout << options << std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("help"))
    {
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("version"))
    {
        std::cout << "Morphology converter " << brion::Version::getString()
                  << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("input") != 1 || vm.count("output") != 1)
    {
        std::cout << options << std::endl;
        return EXIT_FAILURE;
    }

    const brion::URI input(vm["input"].as<std::string>());
    const std::string output(vm["output"].as<std::string>());

    lunchbox::Clock clock;
    const brion::Morphology in(input);
    in.getPoints(); // sync loading
    const float readTime = clock.resetTimef();

    writeMorphology(in, output);
    const float writeTime = clock.resetTimef();

    LBINFO << "Converted " << input << " (" << in.getVersion() << ") => "
           << output << " in " << readTime << " + " << writeTime << " ms"
           << std::endl;
    return EXIT_SUCCESS;
}

namespace
{
void _writeMetadata(H5::H5File& file, const brion::CellFamily& family)
{
    // metadata
    H5::Group metadata = file.createGroup(_g_metadata);

    H5::EnumType familyEnum(H5::PredType::STD_U32LE);
    uint32_t enumValue = brion::FAMILY_NEURON;
    familyEnum.insert("NEURON", &enumValue);
    enumValue = brion::FAMILY_GLIA;
    familyEnum.insert("GLIA", &enumValue);
    familyEnum.commit(metadata, _e_family);

    const hsize_t dim = 1;
    H5::DataSpace familyDS(1, &dim);
    H5::Attribute familyAttr =
        metadata.createAttribute(_a_family, familyEnum, familyDS);
    familyAttr.write(familyEnum, &family);

    const std::string creator = "Brion";
    brion::detail::addStringAttribute(metadata, _a_creator, creator);

    brion::detail::addStringAttribute(metadata, _a_software_version,
                                      BRION_VERSION_STRING);

    const time_t now = ::time(0);
#ifdef _WIN32
    char* gmtString = ::ctime(&now);
#else
    char gmtString[32];
    ::ctime_r(&now, gmtString);
#endif

    std::string creation_time = gmtString;
    creation_time = creation_time.substr(0, creation_time.size() -
                                                1); // ctime_r ends with \n
    brion::detail::addStringAttribute(metadata, _a_creation_time,
                                      creation_time);
    hsize_t dims = 2;
    H5::DataSpace versionDS(1, &dims);
    H5::Attribute versionAttr =
        metadata.createAttribute(_a_version, H5::PredType::STD_U32LE,
                                 versionDS);
    const uint32_t version[2] = {1, 1};
    versionAttr.write(H5::PredType::STD_U32LE, &version[0]);
}

void _writePoints(H5::H5File& file, const brion::Vector4fs& points)
{
    hsize_t dim[2] = {points.size(), 4};
    H5::DataSpace pointsDS(2, dim);

    H5::FloatType pointsDT(H5::PredType::NATIVE_DOUBLE);
    pointsDT.setOrder(H5T_ORDER_LE);

    H5::DataSet dataset = file.createDataSet(_d_points, pointsDT, pointsDS);
    dataset.write(points.data(), H5::PredType::NATIVE_FLOAT);
}

H5::DataSet _getStructureDataSet(H5::H5File& file, const size_t nSections)
{
    H5::DataSet dataset;
    try
    {
        brion::detail::SilenceHDF5 silence;
        return file.openDataSet(_d_structure);
    }
    catch (const H5::Exception&)
    {
        hsize_t dim[2] = {nSections, 3};
        H5::DataSpace structureDS(2, dim);
        H5::FloatType structureDT(H5::PredType::NATIVE_INT);
        structureDT.setOrder(H5T_ORDER_LE);
        return file.createDataSet(_d_structure, structureDT, structureDS);
    }
}

void _writeSections(H5::H5File& file, const brion::Vector2is& sections)
{
    H5::DataSet dataset = _getStructureDataSet(file, sections.size());
    const H5::DataSpace& dspace = dataset.getSpace();
    const hsize_t count[2] = {sections.size(), 1};
    const hsize_t offset[2] = {0, 1};
    dspace.selectHyperslab(H5S_SELECT_XOR, count, offset);

    const hsize_t mdim[2] = {sections.size(), 2};
    const H5::DataSpace mspace(2, mdim);
    dataset.write(sections.data(), H5::PredType::NATIVE_INT, mspace, dspace);
}

void _writeSectionTypes(H5::H5File& file, const brion::SectionTypes& types)
{
    H5::DataSet dataset = _getStructureDataSet(file, types.size());
    const H5::DataSpace& dspace = dataset.getSpace();
    const hsize_t count[2] = {types.size(), 1};
    const hsize_t offset[2] = {0, 1};
    dspace.selectHyperslab(H5S_SELECT_SET, count, offset);

    const hsize_t mdim = types.size();
    const H5::DataSpace mspace(1, &mdim);
    dataset.write(types.data(), H5::PredType::NATIVE_INT, mspace, dspace);
}

void _writePerimeters(H5::H5File& file, const brion::floats& perimeters)
{
    if (perimeters.empty())
        return;

    const hsize_t dim = perimeters.size();
    H5::DataSpace perimeterDS(1, &dim);

    H5::DataSet dataset =
        file.createDataSet(_d_perimeters, H5::PredType::NATIVE_FLOAT,
                           perimeterDS);
    dataset.write(perimeters.data(), H5::PredType::NATIVE_FLOAT);
}
}

void writeMorphology(const brion::Morphology& in, const std::string& output)
{
    H5::H5File file(output, H5F_ACC_TRUNC);
    _writeMetadata(file, in.getCellFamily());
    _writePoints(file, in.getPoints());
    _writeSections(file, in.getSections());
    _writeSectionTypes(file, in.getSectionTypes());
    _writePerimeters(file, in.getPerimeters());
}
