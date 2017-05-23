
/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Mohamed-Ghaith Kaabi <mohamedghaith.kaabi@gmail.com>
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
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 */

#define BOOST_TEST_MODULE brain::CompartmentReportReader

#include <BBP/TestDatasets.h>
#include <brain/compartmentReport.h>
#include <brain/compartmentReportMapping.h>

#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

#define TIMESTEP_PRECISION 0.000005

BOOST_AUTO_TEST_CASE(invalid_open)
{
    BOOST_CHECK_THROW(brain::CompartmentReport(brion::URI("/bla")),
                      std::runtime_error);
    BOOST_CHECK_THROW(brain::CompartmentReport(brion::URI("bla")),
                      std::runtime_error);

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/README";
    BOOST_CHECK_THROW(brain::CompartmentReport(brion::URI(path.string())),
                      std::runtime_error);

    path = BBP_TESTDATA;
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";
    BOOST_CHECK_THROW(brain::CompartmentReport(brion::URI(path.string())),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(open_binary)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/simulations/may17_2011/Control/voltage.bbp";
    BOOST_CHECK_NO_THROW(brain::CompartmentReport(brion::URI(path.string())));
}

BOOST_AUTO_TEST_CASE(open_hdf5)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_NO_THROW(brain::CompartmentReport(brion::URI(path.string())));
}

BOOST_AUTO_TEST_CASE(invalid_mapping)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/simulations/may17_2011/Control/voltage.bbp";
    brion::GIDSet gids;
    gids.insert(123456789);

    brain::CompartmentReport reader(brion::URI(path.string()));

    BOOST_CHECK_THROW(reader.createView(gids), std::runtime_error);
}

void testBounds(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brion::GIDSet gids;
    gids.insert(1);
    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(gids);

    auto frame = view.load(report.getMetaData().startTime).get();
    BOOST_CHECK(frame.data);

    BOOST_CHECK_THROW(frame = view.load(report.getMetaData().endTime).get(),
                      std::logic_error);

    BOOST_CHECK_THROW(frame = view.load(report.getMetaData().endTime + 1).get(),
                      std::logic_error);
}

BOOST_AUTO_TEST_CASE(bounds_binary)
{
    testBounds("local/simulations/may17_2011/Control/voltage.bbp");
}

BOOST_AUTO_TEST_CASE(bounds_hdf5)
{
    testBounds("local/simulations/may17_2011/Control/voltage.h5");
}

inline void testReadSoma(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brion::GIDSet gids;
    gids.insert(1);
    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(gids);

    BOOST_CHECK_EQUAL(report.getMetaData().startTime, 0.);
    BOOST_CHECK_EQUAL(report.getMetaData().endTime, 10.);
    BOOST_CHECK_CLOSE(report.getMetaData().timeStep, 0.1, TIMESTEP_PRECISION);

    auto timestamp = report.getMetaData().startTime;
    auto frame = view.load(timestamp).get();

    BOOST_CHECK(frame.timestamp == timestamp);
    BOOST_CHECK(frame.data);
    BOOST_CHECK_EQUAL((*frame.data)[0], -65);

    frame = view.load(4.5).get();
    BOOST_CHECK(frame.data);
    BOOST_CHECK(frame.timestamp == 4.5);
    BOOST_CHECK_CLOSE((*frame.data)[0], -10.1440039f, .000001f);
}

BOOST_AUTO_TEST_CASE(read_soma_binary)
{
    testReadSoma("local/simulations/may17_2011/Control/voltage.bbp");
}

BOOST_AUTO_TEST_CASE(read_soma_hdf5)
{
    testReadSoma("local/simulations/may17_2011/Control/voltage.h5");
}

inline void testReadAllComps(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(brion::GIDSet());

    BOOST_CHECK_EQUAL(report.getMetaData().startTime, 0.);
    BOOST_CHECK_EQUAL(report.getMetaData().endTime, 10.);
    BOOST_CHECK_CLOSE(report.getMetaData().timeStep, 0.1, TIMESTEP_PRECISION);

    auto frame = view.load(.8f).get();
    BOOST_CHECK(frame.data);
    BOOST_CHECK_CLOSE((*frame.data)[0], -65.2919388, .000001f);
    BOOST_CHECK_CLOSE((*frame.data)[1578], -65.2070618, .000001f);
}

BOOST_AUTO_TEST_CASE(read_allcomps_binary)
{
    testReadAllComps(
        "local/simulations/may17_2011/Control/allCompartments.bbp");
}

BOOST_AUTO_TEST_CASE(read_allcomps_hdf5)
{
    testReadAllComps("local/simulations/may17_2011/Control/allCompartments.h5");
}

void testRead(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brion::GIDSet gids;
    gids.insert(394);
    gids.insert(400);

    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(gids);

    const brion::SectionOffsets& offsets = view.getMapping().getOffsets();
    BOOST_CHECK_EQUAL(offsets.size(), 2);

    BOOST_CHECK_EQUAL(report.getMetaData().startTime, 0.);
    BOOST_CHECK_EQUAL(report.getMetaData().endTime, 10.);
    BOOST_CHECK_CLOSE(report.getMetaData().timeStep, 0.1, TIMESTEP_PRECISION);

    auto frame = view.load(report.getMetaData().startTime).get();

    BOOST_CHECK(frame.data);
    BOOST_CHECK_EQUAL((*frame.data)[offsets[0][0]], -65);
    BOOST_CHECK_EQUAL((*frame.data)[offsets[1][0]], -65);
    BOOST_CHECK_EQUAL((*frame.data)[offsets[0][1]], -65);
    BOOST_CHECK_EQUAL((*frame.data)[offsets[1][1]], -65);

    frame = view.load(4.5f).get();
    BOOST_CHECK(frame.data);
    BOOST_CHECK_CLOSE((*frame.data)[offsets[0][0]], -65.3935928f, .000001f);
    BOOST_CHECK_CLOSE((*frame.data)[offsets[1][0]], -65.9297104f, .000001f);
    BOOST_CHECK_CLOSE((*frame.data)[offsets[0][1]], -65.4166641f, .000001f);
    BOOST_CHECK_CLOSE((*frame.data)[offsets[1][1]], -65.9334106f, .000001f);
}

void testReadRange(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brion::GIDSet gids;
    gids.insert(394);
    gids.insert(400);

    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(gids);

    const double start = report.getMetaData().startTime;
    const double step = report.getMetaData().timeStep;

    auto frames = view.load(start, start + step).get();
    BOOST_REQUIRE_EQUAL(frames.timeStamps->size(), 1);
    BOOST_CHECK_EQUAL((*frames.timeStamps)[0], start);

    frames = view.load(start, start + step * 2).get();
    BOOST_REQUIRE_EQUAL(frames.timeStamps->size(), 2);
    BOOST_CHECK_EQUAL((*frames.timeStamps)[0], start);
    BOOST_CHECK_EQUAL((*frames.timeStamps)[1], start + step);

    frames = view.load(start + 0.5 * step, start + step * 2.5).get();
    BOOST_REQUIRE_EQUAL(frames.timeStamps->size(), 3);
    BOOST_CHECK_EQUAL((*frames.timeStamps)[0], start);
    BOOST_CHECK_EQUAL((*frames.timeStamps)[1], start + step);
    BOOST_CHECK_EQUAL((*frames.timeStamps)[2], start + 2 * step);
}

BOOST_AUTO_TEST_CASE(read_binary)
{
    testRead("local/simulations/may17_2011/Control/allCompartments.bbp");
}

BOOST_AUTO_TEST_CASE(read_hdf5)
{
    testRead("local/simulations/may17_2011/Control/allCompartments.h5");
}

void testReadAll(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brion::GIDSet gids;
    gids.insert(394);
    gids.insert(400);

    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(gids);

    const brion::SectionOffsets& offsets = view.getMapping().getOffsets();
    BOOST_CHECK_EQUAL(offsets.size(), 2);

    BOOST_CHECK_EQUAL(report.getMetaData().startTime, 0.);
    BOOST_CHECK_EQUAL(report.getMetaData().endTime, 10.);
    BOOST_CHECK_CLOSE(report.getMetaData().timeStep, 0.1, TIMESTEP_PRECISION);

    auto frames = view.loadAll().get();

    BOOST_CHECK(frames.timeStamps);
    BOOST_CHECK(frames.data);

    BOOST_CHECK_EQUAL(frames.timeStamps->size(), 100);
}

BOOST_AUTO_TEST_CASE(read_all_binary)
{
    testReadAll("local/simulations/may17_2011/Control/allCompartments.bbp");
}

BOOST_AUTO_TEST_CASE(read_all_hdf5)
{
    testReadAll("local/simulations/may17_2011/Control/allCompartments.h5");
}

void testIndices(const char* relativePath)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= relativePath;

    brion::GIDSet gids{400};

    brain::CompartmentReport report(brion::URI(path.string()));
    auto view = report.createView(gids);

    BOOST_CHECK_EQUAL(view.getMapping().getIndex().size(), 110);
}

BOOST_AUTO_TEST_CASE(indices_hdf5)
{
    testIndices("local/simulations/may17_2011/Control/allCompartments.h5");
}

BOOST_AUTO_TEST_CASE(indices_binary)
{
    testIndices("local/simulations/may17_2011/Control/allCompartments.bbp");
}
