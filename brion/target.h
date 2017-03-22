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

#ifndef BRION_TARGET
#define BRION_TARGET

#include <brion/api.h>
#include <brion/types.h>

namespace brion
{
namespace detail
{
class Target;
}

/** Read access to a Target file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class Target
{
public:
    /** Copy-construct a target file. @version 1.6 */
    BRION_API Target(const Target& from);

    /** Close target file. @version 1.0 */
    BRION_API ~Target();

    /** Assign a different target. @version 1.6 */
    BRION_API Target& operator=(const Target& rhs);

    /** @name Read API */
    //@{
    /** Open the given source to a target file for reading.
     *
     * @param source filepath to target file
     * @throw std::runtime_error if file is not a valid target file
     * @version 1.0
     */
    BRION_API explicit Target(const std::string& source);

    /** Get list of targets for the desired type.
     *
     * @param type desired target type to look for
     * @return list of target names
     * @version 1.0
     */
    BRION_API const Strings& getTargetNames(const TargetType type) const;

    /** @return true if the target exists. @version 1.7 */
    BRION_API bool contains(const std::string& name) const;

    /** Get targets and/or GIDSet grouped by the given target
     *
     * @param name target name to get the values from
     * @return list of values in the target. Can be targets and/or GIDSet
     * @throw std::runtime_error if name is an invalid target
     * @version 1.0
     */
    BRION_API const Strings& get(const std::string& name) const;

    /**
     * Parse a given target into a GID set.
     *
     * All given targets are searched for the given name. If found, the named
     * target is recursively resolved to a GID set.
     * Empty targets are valid, i.e., does not throw when an empty target is
     * found.
     *
     * @param targets the targets to parse
     * @param name the target name to parse
     * @return the set of cell identifiers parsed
     * @throw std::runtime_error if a non-existent (sub)target is given.
     * @version 1.6
     */
    BRION_API static GIDSet parse(const Targets& targets,
                                  const std::string& name);
    //@}

private:
    friend std::ostream& operator<<(std::ostream&, const Target&);

    detail::Target* _impl;
};

/** Stream out content of target file. */
std::ostream& operator<<(std::ostream&, const Target&);
}

#endif
