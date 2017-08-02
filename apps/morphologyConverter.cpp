
/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include <brion/brion.h>

#include <lunchbox/clock.h>
#include <lunchbox/file.h>
#include <lunchbox/log.h>
#include <lunchbox/term.h>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using boost::lexical_cast;

int main(int argc, char* argv[])
{
    po::options_description options(lunchbox::getFilename(argv[0]),
                                    lunchbox::term::getSize().first);

    // clang-format off
    options.add_options()
        ( "help,h", "Produce help message" )
        ( "version,v", "Show program name/version banner and exit" )
        ( "input,i", po::value< std::string >(), "Input morphology" )
        ( "output,o", po::value< std::string >(), "Output morphology" )
        ( "format,f", po::value< std::string >()->default_value( "h5v2" ),
          "Format for output (h5v1, h5v2)" )
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

    const std::string& input =
        vm.count("input") == 1 ? vm["input"].as<std::string>() : std::string();
    const std::string& output = vm.count("output") == 1
                                    ? vm["output"].as<std::string>()
                                    : std::string();

    if (input.empty() || output.empty())
    {
        std::cout << options << std::endl;
        return EXIT_FAILURE;
    }

    brion::MorphologyVersion outVersion = brion::MORPHOLOGY_VERSION_UNDEFINED;
    if (vm["format"].as<std::string>() == "h5v1")
        outVersion = brion::MORPHOLOGY_VERSION_H5_1;

    lunchbox::Clock clock;
    const brion::Morphology in(input);

    float readTime = clock.resetTimef();
    float writeTime = 0.f;

    brion::Morphology out(output, outVersion, true);

    writeTime += clock.resetTimef();
    brion::Vector4fsPtr points = in.readPoints();

    brion::Vector2isPtr sections = in.readSections();
    readTime += clock.resetTimef();

    out.writePoints(*points);
    out.writeSections(*sections);
    writeTime += clock.resetTimef();

    LBDEBUG << points->size() << " points, " << sections->size() << " sections"
            << std::endl;

    brion::SectionTypesPtr types = in.readSectionTypes();
    brion::Vector2isPtr apicals = in.readApicals();
    readTime += clock.resetTimef();

    out.writeSectionTypes(*types);
    if (out.getVersion() != brion::MORPHOLOGY_VERSION_H5_1)
        out.writeApicals(*apicals);

    writeTime += clock.resetTimef();
    LBDEBUG << types->size() << " section types, " << apicals->size()
            << " apicals in " << input << std::endl;

    LBINFO << "Converted " << input << " (" << in.getVersion() << ") => "
           << output << " (" << out.getVersion() << ") in " << readTime << " + "
           << writeTime << " ms" << std::endl;
    return EXIT_SUCCESS;
}
