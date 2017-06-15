/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
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

#include <BBP/TestDatasets.h>
#include <brain/brain.h>
#include <brion/brion.h>

#define BOOST_TEST_MODULE Circuit
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>
#include <lunchbox/bitOperation.h>

namespace
{
std::string getValue(const brion::NeuronMatrix& data, const size_t idx,
                     const uint32_t attr)
{
    return data[idx][lunchbox::getIndexOfLastBit(attr)];
}
}

BOOST_AUTO_TEST_CASE(test_invalid_open)
{
    BOOST_CHECK_THROW(brion::Circuit("/bla"), std::runtime_error);
    BOOST_CHECK_THROW(brion::Circuit("bla"), std::runtime_error);

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/README";
    BOOST_CHECK_THROW(brion::Circuit(path.string()), std::runtime_error);

    path = BBP_TESTDATA;
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW(brion::Circuit(path.string()), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_all_attributes)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/circuits/circuit.mvd2";

    brion::Circuit circuit(path.string());
    BOOST_CHECK_EQUAL(circuit.getNumNeurons(), 10);

    const brion::NeuronMatrix& data =
        circuit.get(brion::GIDSet(), brion::NEURON_ALL_ATTRIBUTES);

    BOOST_CHECK_EQUAL(data.shape()[0], 10); // 10 neurons
    BOOST_CHECK_EQUAL(data.shape()[1], brion::NEURON_ALL);
    BOOST_CHECK_EQUAL(getValue(data, 0, brion::NEURON_MORPHOLOGY_NAME),
                      "R-BJM141005C2_B_cor");
    BOOST_CHECK_EQUAL(getValue(data, 1, brion::NEURON_COLUMN_GID), "0");
    BOOST_CHECK_EQUAL(getValue(data, 6, brion::NEURON_MTYPE), "17");
    BOOST_CHECK_EQUAL(getValue(data, 7, brion::NEURON_POSITION_Y),
                      "399.305168");
}

BOOST_AUTO_TEST_CASE(test_some_attributes)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/circuits/circuit.mvd2";

    brion::Circuit circuit(path.string());
    BOOST_CHECK_EQUAL(circuit.getNumNeurons(), 10);

    brion::GIDSet gids;
    gids.insert(4);
    gids.insert(6);
    const brion::NeuronMatrix& data =
        circuit.get(gids, brion::NEURON_ETYPE | brion::NEURON_MORPHOLOGY_NAME);

    BOOST_CHECK_EQUAL(data.shape()[0], 2); // 2 neurons
    BOOST_CHECK_EQUAL(data.shape()[1], 2); // 2 attributes
    BOOST_CHECK_EQUAL(data[0][0], "L2PC32_2");
    BOOST_CHECK_EQUAL(data[0][1], "0");
    BOOST_CHECK_EQUAL(data[1][0], "R-C010600A2");
    BOOST_CHECK_EQUAL(data[1][1], "3");
}

BOOST_AUTO_TEST_CASE(test_types)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/circuits/18.10.10_600cell/circuit.mvd2";

    brion::Circuit circuit(path.string());
    BOOST_CHECK_EQUAL(circuit.getNumNeurons(), 600);

    const brion::Strings& mtypes = circuit.getTypes(brion::NEURONCLASS_MTYPE);
    BOOST_CHECK_EQUAL(mtypes.size(), 22);
    BOOST_CHECK_EQUAL(mtypes[0], "AHC");
    BOOST_CHECK_EQUAL(mtypes[1], "NGC");
    BOOST_CHECK_EQUAL(mtypes[2], "ADC");
    BOOST_CHECK_EQUAL(mtypes[15], "L4SP");
    BOOST_CHECK_EQUAL(mtypes[21], "L6FFPC");

    const brion::Strings& mclasses =
        circuit.getTypes(brion::NEURONCLASS_MORPHOLOGY_CLASS);
    BOOST_CHECK_EQUAL(mclasses.size(), 22);
    BOOST_CHECK_EQUAL(mclasses[0], "INT");
    BOOST_CHECK_EQUAL(mclasses[1], "INT");
    BOOST_CHECK_EQUAL(mclasses[4], "PYR");
    BOOST_CHECK_EQUAL(mclasses[21], "PYR");

    const brion::Strings& fclasses =
        circuit.getTypes(brion::NEURONCLASS_FUNCTION_CLASS);
    BOOST_CHECK_EQUAL(fclasses.size(), 22);
    BOOST_CHECK_EQUAL(fclasses[0], "INH");
    BOOST_CHECK_EQUAL(fclasses[1], "INH");
    BOOST_CHECK_EQUAL(fclasses[4], "EXC");
    BOOST_CHECK_EQUAL(fclasses[21], "EXC");

    const brion::Strings& etypes = circuit.getTypes(brion::NEURONCLASS_ETYPE);
    BOOST_CHECK_EQUAL(etypes.size(), 8);
    BOOST_CHECK_EQUAL(etypes[0], "cADint");
    BOOST_CHECK_EQUAL(etypes[1], "cFS");
    BOOST_CHECK_EQUAL(etypes[2], "dFS");
    BOOST_CHECK_EQUAL(etypes[3], "cNA");
    BOOST_CHECK_EQUAL(etypes[4], "cADpyr");
    BOOST_CHECK_EQUAL(etypes[5], "bNA");
    BOOST_CHECK_EQUAL(etypes[6], "bAD");
    BOOST_CHECK_EQUAL(etypes[7], "cST");
}

BOOST_AUTO_TEST_CASE(brain_circuit_constructor)
{
    brain::Circuit circuit((brion::URI(bbp::test::getBlueconfig())));
    brain::Circuit circuit2((brion::BlueConfig(bbp::test::getBlueconfig())));
    BOOST_CHECK_THROW(brain::Circuit(brion::URI("pluto")), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(brain_circuit_target)
{
    const brain::Circuit circuit((brion::URI(bbp::test::getBlueconfig())));
    const brion::BlueConfig config(bbp::test::getBlueconfig());

    brion::GIDSet first = circuit.getGIDs();
    brion::GIDSet second = config.parseTarget("Column");
    BOOST_CHECK_EQUAL_COLLECTIONS(first.begin(), first.end(), second.begin(),
                                  second.end());

    first = circuit.getGIDs("Column");
    second = config.parseTarget("Column");
    BOOST_CHECK_EQUAL_COLLECTIONS(first.begin(), first.end(), second.begin(),
                                  second.end());

    first = circuit.getGIDs("Layer1");
    second = config.parseTarget("Layer1");
    BOOST_CHECK_EQUAL_COLLECTIONS(first.begin(), first.end(), second.begin(),
                                  second.end());

    BOOST_CHECK_THROW(circuit.getGIDs("!ThisIsAnInvalidTarget!"),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(brain_circuit_positions)
{
    const brain::Circuit circuit((brion::URI(bbp::test::getBlueconfig())));

    brion::GIDSet gids;
    gids.insert(1);
    gids.insert(2);
    // This call also tests brain::Circuit::getMorphologyURIs
    const brain::Vector3fs positions = circuit.getPositions(gids);
    BOOST_CHECK_EQUAL(positions.size(), gids.size());

    typedef brain::Vector3f V3;
    BOOST_CHECK_SMALL(
        (positions[0] - V3(54.410675, 1427.669280, 124.882234)).length(),
        0.000001f);
    BOOST_CHECK_SMALL(
        (positions[1] - V3(28.758332, 1393.556264, 98.258210)).length(),
        0.000001f);
}

namespace
{
void _checkMorphology(const brain::neuron::Morphology& morphology,
                      const std::string& other)
{
    const brion::Morphology reference(
        BBP_TESTDATA + ("/local/morphologies/01.07.08/h5/" + other));
    BOOST_CHECK(morphology.getPoints() ==
                *reference.readPoints(brion::MORPHOLOGY_UNDEFINED));
}
void _checkMorphology(const brain::neuron::Morphology& morphology,
                      const std::string& other,
                      const brain::Matrix4f& transform)
{
    const brain::neuron::Morphology reference(
        brion::URI(BBP_TESTDATA + ("/local/morphologies/01.07.08/h5/" + other)),
        transform);
    const brain::Vector4fs& p = morphology.getPoints();
    const brain::Vector4fs& q = reference.getPoints();
    BOOST_CHECK(reference.getTransformation().equals(transform));

    BOOST_REQUIRE(p.size() == q.size());
    for (size_t i = 0; i != p.size(); ++i)
        BOOST_CHECK_SMALL((p[i] - q[i]).length(), 0.0001f);
}
}

BOOST_AUTO_TEST_CASE(test_gid_out_of_range)
{
    typedef boost::shared_ptr<const brain::Circuit> CircuitPtr;
    std::vector<CircuitPtr> circuits;
    circuits.push_back(
        CircuitPtr(new brain::Circuit(brion::URI(BBP_TEST_BLUECONFIG))));
#ifdef BRAIN_USE_MVD3
    circuits.push_back(
        CircuitPtr(new brain::Circuit(brion::URI(BBP_TEST_BLUECONFIG3))));
#endif

    brion::GIDSet gids;
    gids.insert(10000000);
    for (const CircuitPtr& circuit : circuits)
    {
        BOOST_CHECK_THROW(circuit->getPositions(gids), std::runtime_error);
        BOOST_CHECK_THROW(circuit->getMorphologyTypes(gids),
                          std::runtime_error);
        BOOST_CHECK_THROW(circuit->getElectrophysiologyTypes(gids),
                          std::runtime_error);
        BOOST_CHECK_THROW(circuit->getRotations(gids), std::runtime_error);
        BOOST_CHECK_THROW(
            circuit->loadMorphologies(gids, brain::Circuit::Coordinates::local),
            std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(load_local_morphologies)
{
    const brain::Circuit circuit((brion::URI(bbp::test::getBlueconfig())));

    brion::GIDSet gids;
    for (uint32_t gid = 1; gid < 500; gid += 75)
        gids.insert(gid);
    // This call also tests brain::Circuit::getMorphologyURIs
    const brain::neuron::Morphologies morphologies =
        circuit.loadMorphologies(gids, brain::Circuit::Coordinates::local);
    BOOST_CHECK_EQUAL(morphologies.size(), gids.size());

    // Checking the first morphology
    _checkMorphology(*morphologies[0], "R-C010306G.h5");

    // Checking shared morphologies
    gids.clear();
    gids.insert(2);
    gids.insert(4);
    gids.insert(6);
    const brain::neuron::Morphologies repeated =
        circuit.loadMorphologies(gids, brain::Circuit::Coordinates::local);

    BOOST_CHECK_EQUAL(repeated.size(), gids.size());
    BOOST_CHECK_EQUAL(repeated[0].get(), repeated[2].get());
    BOOST_CHECK(repeated[0].get() != repeated[1].get());
}

BOOST_AUTO_TEST_CASE(load_global_morphologies)
{
    const brain::Circuit circuit((brion::URI(bbp::test::getBlueconfig())));

    brion::GIDSet gids;
    for (uint32_t gid = 1; gid < 500; gid += 75)
        gids.insert(gid);
    const brain::neuron::Morphologies morphologies =
        circuit.loadMorphologies(gids, brain::Circuit::Coordinates::global);
    BOOST_CHECK_EQUAL(morphologies.size(), gids.size());

    // Checking the first morphology
    brain::Matrix4f matrix;
    matrix.rotate_y(-75.992327 * M_PI / 180.0f);
    matrix.setTranslation(brain::Vector3f(54.410675, 1427.669280, 124.882234));

    _checkMorphology(*morphologies[0], "R-C010306G.h5", matrix);
}

#ifdef BRAIN_USE_MVD3

BOOST_AUTO_TEST_CASE(all_mvd3)
{
    brion::BlueConfig config(BBP_TEST_BLUECONFIG3);
    brain::Circuit circuit(config);
    const size_t numNeurons = circuit.getNumNeurons();
    BOOST_CHECK_EQUAL(circuit.getGIDs().size(), numNeurons);

    brain::Vector3fs positions = circuit.getPositions(circuit.getGIDs());
    brain::Matrix4fs transforms = circuit.getTransforms(circuit.getGIDs());
    BOOST_CHECK_EQUAL(positions.size(), numNeurons);
    BOOST_CHECK_EQUAL(transforms.size(), numNeurons);

    BOOST_CHECK_SMALL((positions[20] - brion::Vector3f(30.1277100000,
                                                       1794.1259110000,
                                                       19.8605870000))
                          .length(),
                      0.000001f);
    BOOST_CHECK_SMALL((positions[100] - brion::Vector3f(48.7579240000,
                                                        1824.4589930000,
                                                        15.3025840000))
                          .length(),
                      0.000001f);

    BOOST_CHECK(transforms[20].equals(
        brain::Matrix4f(brain::Quaternionf(0, 0.923706, 0, 0.383102),
                        brain::Vector3f(30.12771, 1794.125911, 19.860587)),
        0.00001f));
    BOOST_CHECK(transforms[100].equals(
        brain::Matrix4f(brain::Quaternionf(0, -0.992667, 0, 0.120884),
                        brain::Vector3f(48.757924, 1824.458993, 15.302584)),
        0.00001f));
}

BOOST_AUTO_TEST_CASE(partial_mvd3)
{
    brion::BlueConfig config(BBP_TEST_BLUECONFIG3);
    brain::Circuit circuit(config);

    brion::GIDSet gids;
    gids.insert(6);
    gids.insert(21);
    gids.insert(101);
    gids.insert(501);

    const brain::Vector3fs& positions = circuit.getPositions(gids);
    const brain::Matrix4fs& transforms = circuit.getTransforms(gids);
    BOOST_CHECK_EQUAL(positions.size(), 4);
    BOOST_CHECK_EQUAL(transforms.size(), 4);

    BOOST_CHECK_SMALL((positions[1] - brion::Vector3f(30.1277100000,
                                                      1794.1259110000,
                                                      19.8605870000))
                          .length(),
                      0.000001f);
    BOOST_CHECK_SMALL((positions[2] - brion::Vector3f(48.7579240000,
                                                      1824.4589930000,
                                                      15.3025840000))
                          .length(),
                      0.000001f);

    BOOST_CHECK(transforms[1].equals(
        brain::Matrix4f(brain::Quaternionf(0, 0.923706, 0, 0.383102),
                        brain::Vector3f(30.12771, 1794.125911, 19.860587)),
        0.00001f));
    BOOST_CHECK(transforms[2].equals(
        brain::Matrix4f(brain::Quaternionf(0, -0.992667, 0, 0.120884),
                        brain::Vector3f(48.757924, 1824.458993, 15.302584)),
        0.00001f));
}

BOOST_AUTO_TEST_CASE(morphology_names_mvd3)
{
    brion::BlueConfig config(BBP_TEST_BLUECONFIG3);
    brain::Circuit circuit(config);

    brion::GIDSet gids;
    gids.insert(21);
    gids.insert(501);

    const brain::URIs& names = circuit.getMorphologyURIs(gids);
    BOOST_REQUIRE_EQUAL(names.size(), 2);
    BOOST_CHECK(boost::algorithm::ends_with(
        std::to_string(names[0]),
        "dend-C280998A-P3_axon-sm110131a1-3_INT_idA.h5"));
    BOOST_CHECK(
        boost::algorithm::ends_with(std::to_string(names[1]),
                                    "dend-ch160801B_axon-Fluo55_low.h5"));
}

BOOST_AUTO_TEST_CASE(compare_mvd2_mvd3)
{
    brion::BlueConfig config2(BBP_TEST_BLUECONFIG);
    brain::Circuit circuit2(config2);

    brion::BlueConfig config3(BBP_TEST_BLUECONFIG3);
    brain::Circuit circuit3(config3);

    brion::GIDSet gids;
    gids.insert(21);
    gids.insert(501);

    const brain::size_ts& mtypes2 = circuit2.getMorphologyTypes(gids);
    const brain::size_ts& etypes2 = circuit2.getElectrophysiologyTypes(gids);
    const brain::Strings& allMTypes2 = circuit2.getMorphologyTypeNames();
    const brain::Strings& allETypes2 = circuit2.getElectrophysiologyTypeNames();
    const brain::URIs& names2 = circuit2.getMorphologyURIs(gids);

    const brain::size_ts& mtypes3 = circuit3.getMorphologyTypes(gids);
    const brain::size_ts& etypes3 = circuit3.getElectrophysiologyTypes(gids);
    const brain::Strings& allMTypes3 = circuit3.getMorphologyTypeNames();
    const brain::Strings& allETypes3 = circuit3.getElectrophysiologyTypeNames();
    const brain::URIs& names3 = circuit3.getMorphologyURIs(gids);

    BOOST_CHECK_EQUAL_COLLECTIONS(mtypes2.begin(), mtypes2.end(),
                                  mtypes3.begin(), mtypes3.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(etypes2.begin(), etypes2.end(),
                                  etypes3.begin(), etypes3.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(allMTypes2.begin(), allMTypes2.end(),
                                  allMTypes3.begin(), allMTypes3.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(allETypes2.begin(), allETypes2.end(),
                                  allETypes3.begin(), allETypes3.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(names2.begin(), names2.end(), names3.begin(),
                                  names3.end());
}

BOOST_AUTO_TEST_CASE(brain_circuit_random_gids)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::GIDSet& gids = circuit.getRandomGIDs(0.1f);
    BOOST_CHECK_EQUAL(gids.size(), 100);

    const brain::GIDSet& gids2 = circuit.getRandomGIDs(0.1f);
    BOOST_CHECK_EQUAL(gids2.size(), 100);

    bool notEqual = true;
    brain::GIDSet::const_iterator it1 = gids.begin();
    brain::GIDSet::const_iterator it2 = gids2.begin();
    for (; it1 != gids.end(); ++it1, ++it2)
    {
        if (*it1 != *it2)
        {
            notEqual = true;
            break;
        }
    }
    BOOST_CHECK(notEqual);

    const brain::GIDSet& gids3 = circuit.getRandomGIDs(0.5f, "Layer1");
    BOOST_CHECK_EQUAL(gids3.size(), 10);

    BOOST_CHECK_THROW(circuit.getRandomGIDs(-5.f), std::runtime_error);
    BOOST_CHECK_THROW(circuit.getRandomGIDs(1.1f), std::runtime_error);
}

#endif // BRAIN_USE_MVD3
