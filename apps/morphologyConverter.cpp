
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
        ( "stage", po::value< std::string >()->default_value( "all" ),
          "Morphology repair stage to convert (raw, unraveled, repaired, all)" )
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

    typedef std::vector<brion::MorphologyRepairStage> MorphologyRepairStages;
    MorphologyRepairStages stages;
    if (vm["stage"].as<std::string>() == "all")
    {
        stages.push_back(brion::MORPHOLOGY_RAW);
        stages.push_back(brion::MORPHOLOGY_UNRAVELED);
        stages.push_back(brion::MORPHOLOGY_REPAIRED);
    }
    else if (vm["stage"].as<std::string>() == "raw")
        stages.push_back(brion::MORPHOLOGY_RAW);
    else if (vm["stage"].as<std::string>() == "unraveled")
        stages.push_back(brion::MORPHOLOGY_UNRAVELED);
    else if (vm["stage"].as<std::string>() == "repaired")
        stages.push_back(brion::MORPHOLOGY_REPAIRED);

    if (input.empty() || output.empty() || stages.empty())
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
    size_t emptyStages = 0;

    brion::Morphology out(output, outVersion, true);

    if (in.getVersion() == brion::MORPHOLOGY_VERSION_SWC_1 ||
        in.getVersion() == brion::MORPHOLOGY_VERSION_H5_1 ||
        out.getVersion() == brion::MORPHOLOGY_VERSION_H5_1)
    {
        stages.resize(1); // does not have stages and ignores the arg
    }

    BOOST_FOREACH (const brion::MorphologyRepairStage stage, stages)
    {
        writeTime += clock.resetTimef();
        brion::Vector4fsPtr points = in.readPoints(stage);
        if (points->empty())
        {
            ++emptyStages;
            readTime += clock.resetTimef();
            continue;
        }
        brion::Vector2isPtr sections = in.readSections(stage);
        readTime += clock.resetTimef();

        out.writePoints(*points, stage);
        out.writeSections(*sections, stage);
        LBDEBUG << points->size() << " points, " << sections->size()
                << " sections in stage " << lexical_cast<std::string>(stage)
                << std::endl;
    }
    writeTime += clock.resetTimef();

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
           << writeTime << " ms (" << stages.size() - emptyStages << " stages)"
           << std::endl;
    return EXIT_SUCCESS;
}
