/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#ifndef BRION_BLUECONFIG
#define BRION_BLUECONFIG

#include <brion/api.h>
#include <brion/types.h>

#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

namespace brion
{
namespace detail
{
class BlueConfig;
}

/** Read access to a BlueConfig or CircuitConfig file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class BlueConfig : public boost::noncopyable
{
public:
    /** Close BlueConfig or CircuitConfig file. @version 1.0 */
    BRION_API ~BlueConfig();

    /** @name Read API */
    //@{
    /** Open given filepath to a BlueConfig or CircuitConfig for reading.
     *
     * @param source filepath to BlueConfig or CircuitConfig file
     * @throw std::runtime_error if source is not a valid BlueConfig or
     *                           CircuitConfig file
     * @version 1.0
     */
    BRION_API explicit BlueConfig(const std::string& source);

    /** Get names of given section type.
     *
     * @param section type of section to get their names from
     * @return names of sections matching the query type
     * @version 1.0
     */
    BRION_API
    const Strings& getSectionNames(const BlueConfigSection section) const;

    /** Get value as string for key in given section.
     *
     * @param section type of section to get the value from
     * @param sectionName name of section to get the value from
     * @param key name of the key to the value from
     * @return value as string of the given section & key, empty if not found
     * @version 1.0
     */
    BRION_API const std::string& get(const BlueConfigSection section,
                                     const std::string& sectionName,
                                     const std::string& key) const;

    /** Get value as type T for key in given section.
     *
     * @param section type of section to get the value from
     * @param sectionName name of section to get the value from
     * @param key name of the key to the value from
     * @return value as type T of the given section & key
     * @throw boost::bad_lexical_cast if value cannot be casted to type T
     * @version 1.0
     */
    template <typename T>
    T get(const BlueConfigSection section, const std::string& sectionName,
          const std::string& key) const
    {
        const std::string& val = get(section, sectionName, key);
        return val.empty() ? T() : boost::lexical_cast<T>(val);
    }

    /**
     * @return the user and circuit file target parsers.
     * @version 1.7
     */
    BRION_API Targets getTargets() const;
    //@}

    /** @name Semantic read API */
    //@{
    /**
     * @return the URI to the circuit information.
     * @sa Circuit
     * @version 1.7
     */
    BRION_API URI getCircuitSource() const;

    /**
     * @return the URI to the location of synapse nrn files.
     * @version 1.7
     */
    BRION_API URI getSynapseSource() const;

    /**
     * @return the URI to the location of synapse nrn files for projections from
     *         external circuits (e.g. thalamocortical projections)
     * @version 2.0
     */
    BRION_API URI getProjectionSource(const std::string& name) const;

    /**
      * @return the full path to the morphology database. A suffix may be
      *         prepended to the to the bare path from the BlueConfig.
      * @version 1.7
      */
    BRION_API URI getMorphologySource() const;

    /** @return the URI to the named report. @sa CompartmentReport
     * @version 1.7
     */
    BRION_API URI getReportSource(const std::string& report) const;

    /**
     * @return the URI to the default spike data source. @sa SpikeReport
     * @version 1.7
     */
    BRION_API URI getSpikeSource() const;

    /**
     * @return the URIs to the locations of the target files.
     * @version 1.8
     */
    BRION_API URIs getTargetSources() const;

    /**
     * @return the name of the circuit target
     * @version 1.7
     */
    BRION_API std::string getCircuitTarget() const;

    /**
     * @return the set of GIDs for the given target
     * @throw  std::runtime_error if target is invalid
     * @version 1.7
     */
    BRION_API GIDSet parseTarget(const std::string& target) const;

    /** @return the simulation timestep in ms or NaN if undefined.
     * @version 1.7
     */
    BRION_API float getTimestep() const;
    //@}

private:
    friend std::ostream& operator<<(std::ostream&, const BlueConfig&);

    detail::BlueConfig* const _impl;
};

/** Stream out content of BlueConfig or CircuitConfig file. */
std::ostream& operator<<(std::ostream&, const BlueConfig&);
}

#endif
