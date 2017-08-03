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

#ifndef BRION_MORPHOLOGY
#define BRION_MORPHOLOGY

#include <brion/api.h>
#include <brion/types.h>
#include <vmmlib/vector.hpp> // return value

namespace brion
{
/** Read & write access to a Morphology file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class Morphology
{
public:
    /** Close morphology file. @version 1.0 */
    BRION_API ~Morphology();

    /** @name Read API */
    //@{
    /** Open the given source to a morphology file for reading.
     *
     * @param source filepath to morphology file
     * @throw std::runtime_error if file is not a valid morphology file
     * @version 1.0
     */
    BRION_API explicit Morphology(const std::string& source);

    /** @return the cell family of that morphology. @version 1.8 */
    BRION_API CellFamily getCellFamily() const;

    /** Read points of morphology, representing x,y,z coordinates + diameter.
     *
     * @param stage the repair stage to take the points from
     * @return x,y,z coords + diameter of all points of the morphology
     * @version 1.0
     */
    BRION_API Vector4fsPtr
        readPoints(MorphologyRepairStage stage = MORPHOLOGY_UNDEFINED) const;

    /** Read sections of morphology, representing section start index and index
     *  of the parent section.
     *
     * @param stage the repair stage to take the section information from
     * @return First point and parent indices of all the sections of the
     *         morphology.
     * @version 1.0
     */
    BRION_API Vector2isPtr
        readSections(MorphologyRepairStage stage = MORPHOLOGY_UNDEFINED) const;

    /** Read section types of morphology.
     *
     * @return type of all sections of the morphology
     * @version 1.0
     */
    BRION_API SectionTypesPtr readSectionTypes() const;

    /** Read apical points of morphology, representing section and point index.
     *
     * @return section and point index of all apical points in the morphology
     * @version 1.0
     */
    BRION_API Vector2isPtr readApicals() const;

    /**
     * @return perimeters of the cross sections for each point of the morphology
     *         in micrometers.
     * @throw std::runtime_error if empty for FAMILY_GLIA
     * @version 1.8
     */
    BRION_API floatsPtr readPerimeters() const;

    /** @internal */
    BRION_API MorphologyVersion getVersion() const;
    //@}

    /** @name Write API */
    //@{
    /** Open the given morphology file for write access.
     *
     * @param target filepath of the output morphology file
     * @param version the output file format version, automatically chosen if
     *                undefined.
     * @param overwrite true to allow overwrite of existing file
     * @throw std::runtime_error if file could not be created
     * @version 1.0
     * @deprecated
     */
    BRION_API Morphology(const std::string& target, MorphologyVersion version,
                         bool overwrite = false);

    /**
     * Open the given morphology file for write access.
     *
     * @param file filename of the output morphology file
     * @param family the cell family that this morphology represents
     * @throw std::runtime_error if file could not be created
     * @version 1.8
     */
    BRION_API Morphology(const std::string& file, CellFamily family);

    /** Write points of morphology, representing x,y,z coordinates + diameter.
     *
     * @param points x,y,z coords + diameter of all points of the morphology
     * @param stage the repair stage to save those points to
     * @throw std::runtime_error if object not writable
     * @throw std::runtime_error points already written
     * @throw std::runtime_error number of points does not match number of
     *                           perimeters
     * @version 1.0
     */
    BRION_API void writePoints(
        const Vector4fs& points,
        MorphologyRepairStage stage = MORPHOLOGY_UNDEFINED);

    /** Write sections of morphology, representing section start index and index
     *  of parent the section.
     *
     * @param sections index and parent index of all sections of the morphology
     * @param stage the repair stage to save those sections to
     * @throw std::runtime_error if object not writable
     * @version 1.0
     */
    BRION_API void writeSections(
        const Vector2is& sections,
        MorphologyRepairStage stage = MORPHOLOGY_UNDEFINED);

    /** Write section types of morphology.
     *
     * @param types type of each section of the morphology
     * @throw std::runtime_error if object not writable
     * @version 1.0
     */
    BRION_API void writeSectionTypes(const SectionTypes& types);

    /** Write apical points of morphology, representing section and point index.
     *
     * @param apicals section and point index of all apical points
     * @throw std::runtime_error if object not created with write ctor
     * @throw std::runtime_error if not supported by implementation
     * @version 1.0
     * @deprecated
     */
    BRION_API void writeApicals(const Vector2is& apicals);

    /**
     * Write perimeters of morphology.
     *
     * @param perimeters perimeters of the cross sections for each point of
     *                   the morphology in micrometers
     * @throw std::runtime_error if object not writable
     * @throw std::runtime_error perimeters already written
     * @throw std::runtime_error number of points does not match number of
     *                           perimeters
     * @throw std::runtime_error if not supported by implementation
     * @version 1.8
     */
    BRION_API void writePerimeters(const floats& perimeters);

    /** Flush data to output. @version 1.0 */
    BRION_API void flush();
    //@}

private:
    Morphology(const Morphology&) = delete;
    Morphology(Morphology&&) = delete;
    Morphology& operator=(const Morphology&) = delete;
    Morphology& operator=(Morphology&&) = delete;

    class Impl;
    Impl* const _impl;
};
}
#endif
