/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Stefan Eilemann <stefan.eilemann@epfl.ch>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <brion/brion.h>

#include <lunchbox/clock.h>
#include <lunchbox/file.h>
#include <lunchbox/string.h>
#include <lunchbox/term.h>

#ifdef BRION_USE_BBPTESTDATA
#include <BBP/TestDatasets.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>

namespace po = boost::program_options;
using boost::lexical_cast;

#define REQUIRE_EQUAL(a, b)                           \
    if ((a) != (b))                                   \
    {                                                 \
        std::cerr << #a << " != " << #b << std::endl; \
        ::exit(EXIT_FAILURE);                         \
    }

#define REQUIRE(a)                                 \
    if (!(a))                                      \
    {                                              \
        std::cerr << #a << " failed" << std::endl; \
        ::exit(EXIT_FAILURE);                      \
    }

namespace
{
template <class T>
void requireEqualCollections(const T& a, const T& b)
{
    typename T::const_iterator i = a.begin();
    typename T::const_iterator j = b.begin();
    while (i != a.end() && j != b.end())
    {
        REQUIRE_EQUAL(*i, *j);
        ++i;
        ++j;
    }
    REQUIRE_EQUAL(i, a.end());
    REQUIRE_EQUAL(j, b.end());
}
}

/**
 * Convert a compartment report to an HDF5 report.
 *
 * @param argc number of arguments
 * @param argv argument list, use -h for help
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(const int argc, char** argv)
{
    // clang-format off
    po::options_description options("Options",
                                    lunchbox::term::getSize().first);
    options.add_options()
        ("help,h", "Produce help message")
        ("version,v", "Show program name/version banner and exit")
        ("erase,e", po::value< std::string >(),
         "Erase the given report (map-based reports only)")
        ("maxFrames,m", po::value< size_t >(),
         "Convert at most the given number of frames")
        ("gids,g", po::value< std::vector< uint32_t >>()->multitoken(),
         "List of whitespace separated GIDs to convert")
        ("compare,c", "Compare written report with input")
        ("dump,d", "Dump input report information (no output conversion)");

    po::options_description hidden;
    hidden.add_options()
        ("input,i", po::value<std::string>()->required(), "Input report URI")
        ("output,o", po::value<std::string>()->default_value( "dummy://" ),
         "Output report URI");
    // clang-format on

    po::options_description allOptions;
    allOptions.add(hidden).add(options);

    po::positional_options_description positional;
    positional.add("input", 1);
    positional.add("output", 2);

    po::variables_map vm;

    auto parser = po::command_line_parser(argc, argv);
    po::store(parser.options(allOptions).positional(positional).run(), vm);

    if (vm.count("help") || vm.count("input") == 0)
    {
        std::cout << "Usage: " << lunchbox::getFilename(std::string(argv[0]))
                  << " input-uri [output-uri=dummy://] [options]" << std::endl
                  << std::endl
                  << "Supported input and output URIs:" << std::endl
                  << lunchbox::string::prepend(
                         brion::CompartmentReport::getDescriptions(), "    ")
                  << std::endl
#ifdef BRION_USE_BBPTESTDATA
                  << std::endl
                  << "    Test data set (only for input):\n        test:"
                  << std::endl
#endif
                  << std::endl
                  << options << std::endl;
        return EXIT_SUCCESS;
    }
    if (vm.count("version"))
    {
        std::cout << "Brion compartment report converter "
                  << brion::Version::getString() << std::endl;
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

    if (vm.count("erase"))
    {
        lunchbox::URI outURI(vm["erase"].as<std::string>());
        brion::CompartmentReport report(outURI, brion::MODE_READ);
        if (report.erase())
            return EXIT_SUCCESS;
        std::cerr << "Could not erase " << outURI << std::endl;
        return EXIT_FAILURE;
    }

    const size_t maxFrames = vm.count("maxFrames") == 1
                                 ? vm["maxFrames"].as<size_t>()
                                 : std::numeric_limits<size_t>::max();

    std::string input = vm["input"].as<std::string>();
#ifdef BRION_USE_BBPTESTDATA
    if (input == "test:")
    {
        input = std::string(BBP_TESTDATA) +
                "/circuitBuilding_1000neurons/Neurodamus_output/voltages.bbp";
    }
#endif
    if (input == vm["output"].as<std::string>())
    {
        std::cerr << "Cowardly refusing to convert " << input << " onto itself"
                  << std::endl;
        return EXIT_FAILURE;
    }

    lunchbox::URI inURI(input);
    lunchbox::Clock clock;
    brion::CompartmentReport in(inURI, brion::MODE_READ);
    float loadTime = clock.getTimef();

    const double start = in.getStartTime();
    const double step = in.getTimestep();
    double end = in.getEndTime();

    if (vm.count("dump"))
    {
        std::cout << "Compartment report " << inURI << ":" << std::endl
                  << "  " << (end - start) / step << " frames: " << start
                  << ".." << end << " / " << step << " " << in.getTimeUnit()
                  << std::endl
                  << "  " << in.getGIDs().size() << " neurons" << std::endl
                  << "  " << in.getFrameSize() << " compartments" << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("gids"))
    {
        brion::GIDSet gids;
        for (const auto gid : vm["gids"].as<std::vector<uint32_t>>())
            gids.emplace(gid);
        in.updateMapping(gids);
    }

    const double maxEnd = start + maxFrames * step;
    end = std::min(end, maxEnd);
    const brion::CompartmentCounts& counts = in.getCompartmentCounts();
    const brion::GIDSet& gids = in.getGIDs();

    lunchbox::URI outURI(vm["output"].as<std::string>());
    if (outURI.getPath().empty())
    {
        try
        {
            outURI.setPath(
                boost::filesystem::canonical(inURI.getPath()).generic_string());
        }
        catch (const boost::filesystem::filesystem_error&)
        {
            // For non-filebased reports, the canonical above will throw.
            outURI.setPath(inURI.getPath());
        }
    }

    clock.reset();
    brion::CompartmentReport to(outURI, brion::MODE_OVERWRITE);
    to.writeHeader(start, end, step, in.getDataUnit(), in.getTimeUnit());
    {
        size_t index = 0;
        for (const uint32_t gid : gids)
            if (!to.writeCompartments(gid, counts[index++]))
                return EXIT_FAILURE;
    }

    float writeTime = clock.getTimef();

    // Adding step / 2 to the window to avoid off by 1 errors during truncation
    const size_t nFrames = (end - start + step * 0.5) / step;

    boost::progress_display progress(nFrames);

    for (size_t frameIndex = 0; frameIndex < nFrames; ++frameIndex)
    {
        // Making the timestamp fall in the middle of the frame
        const double t = start + frameIndex * step + step * 0.5;

        clock.reset();

        brion::floatsPtr data;
        try
        {
            data = in.loadFrame(t).get();
        }
        catch (...)
        {
            LBERROR << std::endl
                    << "Can't load frame at " << t << " ms" << std::endl;
            ::exit(EXIT_FAILURE);
        }
        loadTime += clock.getTimef();
        if (!data)
        {
            LBERROR << std::endl
                    << "Can't load frame at " << t << " ms" << std::endl;
            ::exit(EXIT_FAILURE);
        }

        const brion::floats& values = *data.get();
        const auto& offsets = in.getOffsets();

        size_t index = 0;
        clock.reset();
        for (const uint32_t gid : gids)
        {
            const float* cellValues = &values[offsets[index][0]];
            const size_t size =
                std::accumulate(counts[index].begin(), counts[index].end(), 0);
            if (!to.writeFrame(gid, cellValues, size, t))
                return EXIT_FAILURE;
            ++index;
            continue;
        }
        writeTime += clock.getTimef();
        ++progress;
    }

    clock.reset();
    to.flush();

    writeTime += clock.getTimef();

    std::cout << "Converted " << inURI << " to " << outURI << " (in "
              << size_t(loadTime) << " out " << size_t(writeTime) << " ms, "
              << gids.size() << " cells X " << nFrames << " frames)"
              << std::endl;

    if (vm.count("compare"))
    {
        progress.restart(nFrames);
        brion::CompartmentReport result(outURI, brion::MODE_READ);

        REQUIRE_EQUAL(in.getStartTime(), result.getStartTime());
        REQUIRE_EQUAL(in.getEndTime(), result.getEndTime());
        REQUIRE_EQUAL(in.getTimestep(), result.getTimestep());
        REQUIRE_EQUAL(in.getFrameSize(), result.getFrameSize());
        requireEqualCollections(gids, result.getGIDs());
        REQUIRE_EQUAL(in.getDataUnit(), result.getDataUnit());
        REQUIRE_EQUAL(in.getTimeUnit(), result.getTimeUnit());
        REQUIRE(!in.getDataUnit().empty());
        REQUIRE(!in.getTimeUnit().empty());

        const brion::SectionOffsets& offsets1 = in.getOffsets();
        const brion::SectionOffsets& offsets2 = result.getOffsets();
        const brion::CompartmentCounts& counts1 = in.getCompartmentCounts();
        const brion::CompartmentCounts& counts2 = result.getCompartmentCounts();

        REQUIRE_EQUAL(offsets1.size(), offsets2.size());
        REQUIRE_EQUAL(counts1.size(), counts2.size());

        for (size_t i = 0; i < offsets1.size(); ++i)
        {
            requireEqualCollections(offsets1[i], offsets2[i]);

            for (size_t j = 0; j < offsets1[i].size(); ++j)
                REQUIRE(offsets1[i][j] < in.getFrameSize() ||
                        offsets1[i][j] == std::numeric_limits<uint64_t>::max());
        }

        for (size_t frameIndex = 0; frameIndex < nFrames; ++frameIndex)
        {
            // Making the timestamp fall in the middle of the frame
            const double t = start + frameIndex * step + step * 0.5;
            brion::floatsPtr frame1 = in.loadFrame(t).get();
            brion::floatsPtr frame2 = result.loadFrame(t).get();

            REQUIRE(frame1);
            REQUIRE(frame2);

            for (size_t i = 0; i < in.getFrameSize(); ++i)
                REQUIRE_EQUAL((*frame1)[i], (*frame2)[i]);

            ++progress;
        }
    }

    return EXIT_SUCCESS;
}
