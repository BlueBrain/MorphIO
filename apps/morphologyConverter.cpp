
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include <brion/brion.h>
#include <brion/detail/morphologyHDF5.h>
#include <brion/detail/utilsHDF5.h>

#include <lunchbox/clock.h>
#include <lunchbox/file.h>
#include <lunchbox/log.h>
#include <lunchbox/term.h>

#include <H5Cpp.h>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <H5Tpublic.h>
#include <highfive/H5Attribute.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>

namespace HighFive
{
class UserType : public DataType
{
public:
    UserType(hid_t hid) { _hid = hid; }
};
}

namespace po = boost::program_options;
using boost::lexical_cast;
void writeMorphology(const brion::Morphology& in, const std::string& output);

int main(int argc, char* argv[])
{
    po::options_description options(lunchbox::getFilename(argv[0]),
                                    lunchbox::term::getSize().first);

    // clang-format off
    options.add_options()("help,h", "Produce help message")(
        "version,v", "Show program name/version banner and exit");

    po::options_description hidden;
    hidden.add_options()
        ("input,i", po::value<std::string>()->required(), "Input morphology")
        ("output,o", po::value<std::string>()->required(),
         "Output H5 V1.1 morphology");
    // clang-format on

    po::options_description allOptions;
    allOptions.add(hidden).add(options);

    po::positional_options_description positional;
    positional.add("input", 1);
    positional.add("output", 2);

    po::variables_map vm;

    auto parser = po::command_line_parser(argc, argv);
    po::store(parser.options(allOptions).positional(positional).run(), vm);

    if (vm.count("help") || vm.count("input") == 0 || vm.count("output") == 0)
    {
        std::cout << "Usage: " << lunchbox::getFilename(std::string(argv[0]))
                  << " input output" << std::endl
                  << std::endl;
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("version"))
    {
        std::cout << "Morphology converter " << brion::Version::getString()
                  << std::endl;
        return EXIT_SUCCESS;
    }

    try
    {
        po::notify(vm);
    }
    catch (const po::error& e)
    {
        std::cerr << "Command line parse error: " << e.what() << std::endl
                  << options << std::endl;
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
void _writeMetadata(HighFive::File& file, const brion::CellFamily& family)
{
    // metadata for v 1.1
    auto metadata = file.createGroup(_g_metadata);

    // HighFive doesn't provide an API for enums
    auto familyEnum = HighFive::UserType(H5Tenum_create(H5T_NATIVE_INT));
    auto enumValue = brion::FAMILY_NEURON;
    H5Tenum_insert(familyEnum.getId(), "NEURON", &enumValue);
    enumValue = brion::FAMILY_GLIA;
    H5Tenum_insert(familyEnum.getId(), "GLIA", &enumValue);

    auto familyAttr =
        metadata.createAttribute(_a_family,
                                 HighFive::DataSpace(std::vector<size_t>({1})),
                                 familyEnum);
    H5Awrite(familyAttr.getId(), familyEnum.getId(), &family);

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

    auto version = metadata.createAttribute<uint32_t>(
        _a_version, HighFive::DataSpace(std::vector<size_t>({2})));
    version.write(std::vector<uint32_t>{1, 1});
}

template <typename T>
void _writePoints(HighFive::NodeTraits<T>& group,
                  const brion::Vector4fs& points)
{
    auto dspace = HighFive::DataSpace({points.size(), 4});
    // I've confirmed that other tools in the moprhology toolchain write
    // doubles here (for no good reason).
    auto dataset = group.template createDataSet<double>(_d_points, dspace);
    dataset.write(points);
}

void _writeStructure(HighFive::File& file, const brion::Vector2is& sections,
                     const brion::SectionTypes& types)
{
    assert(sections.size() == types.size());
    auto dataset =
        file.createDataSet<int>(_d_structure,
                                HighFive::DataSpace({sections.size(), 3}));

    auto slab = dataset.select({0, 0}, {sections.size(), 2}, {1, 2});
    slab.write(sections);

    slab = dataset.select({0, 1}, {types.size(), 1});
    slab.write(types);
}

void _writePerimeters(HighFive::File& file, const brion::floats& perimeters)
{
    if (perimeters.empty())
        return;

    const auto dspace =
        HighFive::DataSpace(std::vector<size_t>({perimeters.size()}));
    auto dataset = file.createDataSet<float>(_d_perimeters, dspace);
    dataset.write(perimeters);
}
}

void writeMorphology(const brion::Morphology& in, const std::string& output)
{
    HighFive::File file(output,
                        HighFive::File::Create | HighFive::File::Truncate);

    _writeMetadata(file, in.getCellFamily());
    _writePoints(file, in.getPoints());
    _writeStructure(file, in.getSections(), in.getSectionTypes());
    _writePerimeters(file, in.getPerimeters());
}
