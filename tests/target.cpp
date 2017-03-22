/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#include <BBP/TestDatasets.h>
#include <brion/brion.h>

#define BOOST_TEST_MODULE Target
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(invalid_open)
{
    BOOST_CHECK_THROW(brion::Target("blub"), std::runtime_error);

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/README";
    BOOST_CHECK_THROW(brion::Target(path.string()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(get)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/start.target";

    const brion::Target target(path.string());
    std::cout << target << std::endl;

    const brion::Strings& targets = target.getTargetNames(brion::TARGET_CELL);
    BOOST_CHECK_EQUAL(targets.size(), 46);
    BOOST_CHECK_EQUAL(target.getTargetNames(brion::TARGET_COMPARTMENT).size(),
                      0);

    const brion::Strings& columnTarget = target.get(targets[0]);
    BOOST_CHECK_EQUAL(columnTarget.size(), 6);
    BOOST_CHECK_EQUAL(columnTarget[0], "Layer1");

    const brion::Strings& layer4Target = target.get(targets[4]);
    BOOST_CHECK_EQUAL(layer4Target.size(), 124);
    BOOST_CHECK_EQUAL(layer4Target[0], "a269");
    BOOST_CHECK_EQUAL(layer4Target[10], "a279");
    BOOST_CHECK_EQUAL(layer4Target[42], "a311");
}

BOOST_AUTO_TEST_CASE(parse)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/circuits/18.10.10_600cell/ncsFunctionalCompare/start.target";
    const brion::Target target(path.string());
    const brion::GIDSet& column =
        brion::Target::parse(brion::Targets(1, target), "Column");
    BOOST_CHECK_EQUAL(column.size(), 600);
}

BOOST_AUTO_TEST_CASE(parseBroken)
{
    boost::filesystem::path pathUser(BBP_TEST_USER_TARGET);
    boost::filesystem::path pathStart(BBP_TEST_START_TARGET);

    const brion::Target startTarget(pathUser.string());
    const brion::Target userTarget(pathStart.string());

    brion::Targets testTargets;
    testTargets.push_back(startTarget);
    testTargets.push_back(userTarget);

    const brion::GIDSet& column = brion::Target::parse(testTargets, "Column");

    BOOST_CHECK_NO_THROW(brion::Target::parse(testTargets, "Column"));
    BOOST_CHECK_NO_THROW(brion::Target::parse(testTargets, "EmptyColumn"));
    BOOST_CHECK_NO_THROW(brion::Target::parse(testTargets, "EmptyTarget"));
    BOOST_CHECK_THROW(brion::Target::parse(testTargets, "BrokenColumn"),
                      std::runtime_error);

    BOOST_CHECK_EQUAL(column.size(), 1000);
}
