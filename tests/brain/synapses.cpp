/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
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
#include <vmmlib/vmmlib.hpp>

#define BOOST_TEST_MODULE Synapses
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(projection)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& syn1 =
        circuit.getProjectedSynapses(circuit.getGIDs("Layer1"),
                                     circuit.getGIDs("Layer2"));
    const brain::Synapses& syn2 =
        circuit.getProjectedSynapses(circuit.getGIDs("Layer2"),
                                     circuit.getGIDs("Layer1"));
    BOOST_CHECK_NE(syn1.size(), syn2.size());
    BOOST_CHECK_EQUAL(syn1.size(), 895);
    BOOST_CHECK_EQUAL(syn2.size(), 353);
    BOOST_CHECK_EQUAL(syn1[100].getPresynapticGID(), 3);
    BOOST_CHECK_EQUAL(syn1[100].getPostsynapticGID(), 141);
    BOOST_CHECK_EQUAL(syn2[100].getPresynapticGID(), 115);
    BOOST_CHECK_EQUAL(syn2[100].getPostsynapticGID(), 7);
}

BOOST_AUTO_TEST_CASE(projection_stream)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    brain::SynapsesStream stream =
        circuit.getProjectedSynapses(circuit.getGIDs("Layer2"),
                                     circuit.getGIDs("Layer5"),
                                     brain::SynapsePrefetch::positions);
    const size_t remaining = 130;
    BOOST_CHECK_EQUAL(stream.getRemaining(), remaining);
    std::future<brain::Synapses> future = stream.read();
    size_t i = 1;
    size_t totalSize = 0;
    vmml::AABBf bbox;
    while (!stream.eos())
    {
        const brain::Synapses synapses = future.get();
        future = stream.read(); // fetch next

        ++i;
        BOOST_CHECK_EQUAL(stream.getRemaining(), remaining - i);

        const float* __restrict__ posx = synapses.preSurfaceXPositions();
        const float* __restrict__ posy = synapses.preSurfaceYPositions();
        const float* __restrict__ posz = synapses.preSurfaceZPositions();

        for (size_t j = 0; j < synapses.size(); ++j)
            bbox.merge(vmml::Vector3f(posx[j], posy[j], posz[j]));
        totalSize += synapses.size();
    }

    BOOST_CHECK_CLOSE(bbox.getCenter()[0], 19.4931183f, 0.00001f);
    BOOST_CHECK_CLOSE(bbox.getCenter()[1], 1384.17578f, 0.00001f);
    BOOST_CHECK_CLOSE(bbox.getCenter()[2], 18.0030212f, 0.00001f);
    BOOST_CHECK_EQUAL(totalSize, 9520);
}

BOOST_AUTO_TEST_CASE(afferent_synapses)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getAfferentSynapses(circuit.getGIDs("Layer1"),
                                    brain::SynapsePrefetch::all);
    BOOST_CHECK(!synapses.empty());
    BOOST_CHECK_EQUAL(synapses.size(), 1172);
    BOOST_CHECK_EQUAL(synapses[0].getPresynapticGID(), 10);
    BOOST_CHECK_CLOSE(synapses[1].getPostsynapticDistance(), 1.34995711f,
                      0.00001f);
    BOOST_CHECK_CLOSE(synapses[2].getConductance(), 0.34758395f, 0.00001f);
    BOOST_CHECK_THROW(synapses[3].getGID(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(efferent_synapses)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getEfferentSynapses(brion::GIDSet{10},
                                    brain::SynapsePrefetch::all);
    BOOST_CHECK(!synapses.empty());
    BOOST_CHECK_EQUAL(synapses.size(), 74);
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticGID(), 1);
    BOOST_CHECK_CLOSE(synapses[1].getPostsynapticDistance(), 1.34995711f,
                      0.00001f);
    BOOST_CHECK_CLOSE(synapses[2].getConductance(), 0.34758395f, 0.00001f);
    BOOST_CHECK_THROW(synapses[3].getGID(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(retrograde_projection)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brion::GIDSet& preNeurons = circuit.getGIDs("Layer1");
    const brion::GIDSet postNeuron = {1};
    const brain::Synapses& synapses =
        circuit.getProjectedSynapses(preNeurons, postNeuron,
                                     brain::SynapsePrefetch::all);
    BOOST_CHECK(!synapses.empty());
    BOOST_CHECK_EQUAL(synapses.size(), 5);
    for (const auto& synapse : synapses)
        BOOST_CHECK_EQUAL(synapse.getPresynapticGID(), 10);
}

BOOST_AUTO_TEST_CASE(lazy_loading_afferent)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getAfferentSynapses(circuit.getGIDs("Layer1"),
                                    brain::SynapsePrefetch::all);
    const brain::Synapses& synapsesLazy =
        circuit.getAfferentSynapses(circuit.getGIDs("Layer1"));
    BOOST_CHECK_EQUAL(synapses.size(), synapsesLazy.size());
    BOOST_CHECK_EQUAL(synapses[0].getPresynapticGID(),
                      synapsesLazy[0].getPresynapticGID());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticDistance(),
                      synapsesLazy[0].getPostsynapticDistance());
    BOOST_CHECK_EQUAL(synapses[0].getConductance(),
                      synapsesLazy[0].getConductance());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticCenterPosition(),
                      synapsesLazy[0].getPostsynapticCenterPosition());
}

BOOST_AUTO_TEST_CASE(lazy_loading_efferent)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getEfferentSynapses(circuit.getGIDs("Layer1"),
                                    brain::SynapsePrefetch::all);
    const brain::Synapses& synapsesLazy =
        circuit.getEfferentSynapses(circuit.getGIDs("Layer1"));

    BOOST_CHECK_EQUAL(synapses.size(), synapsesLazy.size());
    BOOST_CHECK_EQUAL(synapses[0].getPresynapticGID(),
                      synapsesLazy[0].getPresynapticGID());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticDistance(),
                      synapsesLazy[0].getPostsynapticDistance());
    BOOST_CHECK_EQUAL(synapses[0].getConductance(),
                      synapsesLazy[0].getConductance());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticCenterPosition(),
                      synapsesLazy[0].getPostsynapticCenterPosition());
}

BOOST_AUTO_TEST_CASE(lazy_loading_pathway)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getProjectedSynapses(circuit.getGIDs("Layer2"),
                                     circuit.getGIDs("Layer4"),
                                     brain::SynapsePrefetch::all);
    const brain::Synapses& synapsesLazy =
        circuit.getProjectedSynapses(circuit.getGIDs("Layer2"),
                                     circuit.getGIDs("Layer4"));

    BOOST_CHECK_EQUAL(synapses.size(), synapsesLazy.size());
    BOOST_CHECK_EQUAL(synapses[0].getPresynapticGID(),
                      synapsesLazy[0].getPresynapticGID());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticDistance(),
                      synapsesLazy[0].getPostsynapticDistance());
    BOOST_CHECK_EQUAL(synapses[0].getConductance(),
                      synapsesLazy[0].getConductance());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticCenterPosition(),
                      synapsesLazy[0].getPostsynapticCenterPosition());
}

BOOST_AUTO_TEST_CASE(copy)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getAfferentSynapses(circuit.getGIDs("Layer1"));

    const brain::Synapses synapsesCopy = synapses;

    BOOST_CHECK_EQUAL(synapses.size(), synapsesCopy.size());
    BOOST_CHECK_EQUAL(synapses[0].getPresynapticGID(),
                      synapsesCopy[0].getPresynapticGID());

    const brain::Synapse synapse = synapses[1];
    const brain::Synapse synapseCopy = synapse;
    BOOST_CHECK_EQUAL(synapse.getPresynapticGID(),
                      synapseCopy.getPresynapticGID());

    BOOST_CHECK_EQUAL(synapse.getDepression(), synapseCopy.getDepression());
}

BOOST_AUTO_TEST_CASE(full_copy)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getAfferentSynapses(circuit.getGIDs("Layer1"),
                                    brain::SynapsePrefetch::all);

    const brain::Synapses synapsesCopy = synapses;

    BOOST_CHECK_EQUAL(synapses.size(), synapsesCopy.size());
    BOOST_CHECK_EQUAL(synapses[0].getPostsynapticSurfacePosition(),
                      synapsesCopy[0].getPostsynapticSurfacePosition());
    BOOST_CHECK_EQUAL(synapses[10].getEfficacy(),
                      synapsesCopy[10].getEfficacy());
}

BOOST_AUTO_TEST_CASE(check_all_synapse_attributes)
{
    const brain::Circuit circuit(brion::URI(BBP_TEST_BLUECONFIG3));
    const brain::Synapses& synapses =
        circuit.getAfferentSynapses(brion::GIDSet{1},
                                    brain::SynapsePrefetch::all);
    BOOST_CHECK_EQUAL(synapses.size(), 77);

    const brain::Synapse& synapse = synapses[0];
    BOOST_CHECK_EQUAL(synapse.getConductance(), 0.572888553f);
    BOOST_CHECK_EQUAL(synapse.getDecay(), 10.208410263f);
    BOOST_CHECK_EQUAL(synapse.getDelay(), 0.583546519f);
    BOOST_CHECK_EQUAL(synapse.getDepression(), 1057);
    BOOST_CHECK_EQUAL(synapse.getEfficacy(), 0);
    BOOST_CHECK_EQUAL(synapse.getFacilitation(), 29);
    BOOST_CHECK_THROW(synapse.getGID(), std::runtime_error);
    BOOST_CHECK_EQUAL(synapse.getPostsynapticCenterPosition(),
                      brion::Vector3f(3.799289703f, 1947.041748047f,
                                      9.237932205f));
    BOOST_CHECK_EQUAL(synapse.getPostsynapticDistance(), 0.924134851f);
    BOOST_CHECK_EQUAL(synapse.getPostsynapticGID(), 1);
    BOOST_CHECK_EQUAL(synapse.getPostsynapticSectionID(), 70);
    BOOST_CHECK_EQUAL(synapse.getPostsynapticSegmentID(), 13);
    BOOST_CHECK_EQUAL(synapse.getPostsynapticSurfacePosition(),
                      brion::Vector3f(3.603360415f, 1947.145141602f,
                                      9.205502510f));
    BOOST_CHECK_EQUAL(synapse.getPresynapticCenterPosition(),
                      brion::Vector3f(3.611587524f, 1947.084228516f,
                                      9.198493958f));
    BOOST_CHECK_EQUAL(synapse.getPresynapticDistance(), 2.911511898f);
    BOOST_CHECK_EQUAL(synapse.getPresynapticGID(), 10);
    BOOST_CHECK_EQUAL(synapse.getPresynapticSectionID(), 2);
    BOOST_CHECK_EQUAL(synapse.getPresynapticSegmentID(), 15);
    BOOST_CHECK_EQUAL(synapse.getPresynapticSurfacePosition(),
                      brion::Vector3f(3.792815685f, 1947.050537109f,
                                      9.214178085f));
    BOOST_CHECK_EQUAL(synapse.getUtilization(), 0.362769693f);
}
