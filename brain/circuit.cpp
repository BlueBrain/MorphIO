/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *                          Adrien.Devresse@epfl.ch
 *                          Daniel.Nachbaur@epfl.ch
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
 */

#include "circuit.h"
#include "detail/circuit.h"

#include "synapsesStream.h"
#include <boost/algorithm/string.hpp>

namespace brain
{
Circuit::Impl* newImpl(const brion::BlueConfig& config)
{
    const std::string circuit = config.getCircuitSource().getPath();
    if (boost::algorithm::ends_with(circuit, ".mvd2"))
        return new MVD2(config);
#ifdef BRAIN_USE_MVD3
    return new MVD3(config);
#else
    throw std::runtime_error("MVD3 support requires CMake 3");
#endif
}

Circuit::Circuit(const URI& source)
    : _impl(newImpl(brion::BlueConfig(source.getPath())))
{
}

Circuit::Circuit(const brion::BlueConfig& config)
    : _impl(newImpl(config))
{
}

Circuit::~Circuit()
{
}

GIDSet Circuit::getGIDs() const
{
    return _impl->getGIDs();
}

GIDSet Circuit::getGIDs(const std::string& target) const
{
    return _impl->getGIDs(target);
}

GIDSet Circuit::getRandomGIDs(const float fraction) const
{
    return _impl->getRandomGIDs(fraction, "");
}

GIDSet Circuit::getRandomGIDs(const float fraction,
                              const std::string& target) const
{
    return _impl->getRandomGIDs(fraction, target);
}

URIs Circuit::getMorphologyURIs(const GIDSet& gids) const
{
    const Strings& names = _impl->getMorphologyNames(gids);

    URIs uris;
    uris.reserve(names.size());
    for (Strings::const_iterator i = names.begin(); i < names.end(); ++i)
        uris.push_back(_impl->getMorphologyURI(*i));
    return uris;
}

neuron::Morphologies Circuit::loadMorphologies(const GIDSet& gids,
                                               const Coordinates coords) const
{
    URIs uris = getMorphologyURIs(gids);
    const auto circuitPath =
        // cache outside of loop, canonical does stat() which is slow on GPFS
        fs::canonical(_impl->getCircuitSource().getPath()).generic_string();
    const bool transform = coords == Coordinates::global;

    // < GID, hash >
    Strings hashes;
    hashes.reserve(uris.size());
    std::set<std::string> hashSet;
    GIDSet::const_iterator gid = gids.begin();
    for (auto& uri : uris)
    {
        // opt: don't stat abs file path (see above)
        if (uri.getScheme() == "file" && uri.getPath()[0] != '/')
            uri.setPath(fs::canonical(uri.getPath()).generic_string());

        auto hash = uri.getPath();
        if (transform)
            // store circuit + GID for transformed morphology
            hash += circuitPath + std::to_string(*gid);

        hash = servus::make_uint128(hash + "v2").getString();
        hashes.push_back(hash);
        hashSet.insert(hash);
        ++gid;
    }

    CachedMorphologies cached = _impl->loadMorphologiesFromCache(hashSet);
    using MorphologyUse = std::pair<size_t, brion::MorphologyPtr>;
    std::unordered_map<std::string, MorphologyUse> loading;

    // resolve missing morphologies
    for (size_t i = 0; i < uris.size(); ++i)
    {
        const URI& uri = uris[i];
        const std::string& hash = hashes[i];

        if (cached.find(hash) == cached.end())
        {
            const auto& key = std::to_string(uri);
            auto load = loading.find(key);
            if (load == loading.end())
            { // delay data access to next loop to allow async loading
                loading.insert(std::make_pair(
                    key, std::make_pair(1, std::make_shared<brion::Morphology>(
                                               uri))));
            }
            else
                ++load->second.first;
            cached.insert(std::make_pair(hash, nullptr));
        }
    }

    // load and transform missing and put them in GID-order into result
    neuron::Morphologies result;
    result.reserve(uris.size());
    const Matrix4fs transforms = transform ? getTransforms(gids) : Matrix4fs();

    for (size_t i = 0; i < uris.size(); ++i)
    {
        const URI& uri = uris[i];
        const std::string& hash = hashes[i];

        auto it = cached.find(hash);
        if (it->second)
            result.push_back(it->second);
        else
        {
            const auto& key = std::to_string(uri);
            auto& load = loading.find(key)->second;
            --load.first;

            neuron::MorphologyPtr morphology;
            if (transform)
            {
                if (load.first == 0) // last usage, take existing instance
                    morphology.reset(
                        new neuron::Morphology(load.second, transforms[i]));
                else // make a copy
                    morphology.reset(new neuron::Morphology(
                        std::make_shared<brion::Morphology>(*load.second),
                        transforms[i]));
            }
            else
                // share unmodified brion data
                morphology.reset(new neuron::Morphology(load.second));

            _impl->saveMorphologyToCache(uri.getPath(), hash, morphology);
            it->second = morphology;
            result.push_back(morphology);
        }
    }

    return result;
}

Vector3fs Circuit::getPositions(const GIDSet& gids) const
{
    return _impl->getPositions(gids);
}

size_ts Circuit::getMorphologyTypes(const GIDSet& gids) const
{
    return _impl->getMTypes(gids);
}

Strings Circuit::getMorphologyTypeNames() const
{
    return _impl->getMorphologyNames();
}

size_ts Circuit::getElectrophysiologyTypes(const GIDSet& gids) const
{
    return _impl->getETypes(gids);
}

Strings Circuit::getElectrophysiologyTypeNames() const
{
    return _impl->getElectrophysiologyNames();
}

Matrix4fs Circuit::getTransforms(const GIDSet& gids) const
{
    const Vector3fs& positions = _impl->getPositions(gids);
    const Quaternionfs& rotations = _impl->getRotations(gids);
    if (positions.size() != rotations.size())
        throw std::runtime_error(
            "Positions not equal rotations for given GIDs");

    Matrix4fs transforms(positions.size());

#pragma omp parallel for
    for (size_t i = 0; i < positions.size(); ++i)
        transforms[i] = Matrix4f(rotations[i], positions[i]);
    return transforms;
}

Quaternionfs Circuit::getRotations(const GIDSet& gids) const
{
    return _impl->getRotations(gids);
}

size_t Circuit::getNumNeurons() const
{
    return _impl->getNumNeurons();
}

SynapsesStream Circuit::getAfferentSynapses(
    const GIDSet& gids, const SynapsePrefetch prefetch) const
{
    return SynapsesStream(*this, gids, true, prefetch);
}

SynapsesStream Circuit::getExternalAfferentSynapses(
    const GIDSet& gids, const std::string& source,
    const SynapsePrefetch prefetch) const
{
    return SynapsesStream(*this, gids, source, prefetch);
}

SynapsesStream Circuit::getEfferentSynapses(
    const GIDSet& gids, const SynapsePrefetch prefetch) const
{
    return SynapsesStream(*this, gids, false, prefetch);
}

SynapsesStream Circuit::getProjectedSynapses(
    const GIDSet& preGIDs, const GIDSet& postGIDs,
    const SynapsePrefetch prefetch) const
{
    return SynapsesStream(*this, preGIDs, postGIDs, prefetch);
}
}
