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
#include <servus/serializable.h> // return value
#include <vmmlib/vector.hpp>     // return value

namespace brion
{
/** Read access a Morphology file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class Morphology
{
public:
    /** Close morphology file. @version 1.0 */
    BRION_API ~Morphology();

    BRION_API Morphology(const void* data, size_t size);
    BRION_API Morphology(const Morphology&);
    BRION_API Morphology& operator=(const Morphology&);
    BRION_API Morphology(Morphology&&);
    BRION_API Morphology& operator=(Morphology&&);

    /** @name Read API */
    //@{
    /** Open the given source to a morphology file and parse it.
     *
     * The actual data loading happens in a background thread and is lazily
     * finalised in any get method. It is therefore faster to construct a set
     * of morphologies and then read their data over a serial construct-and-read
     * approach.
     *
     * @param source URI to load the morphology
     * @throw std::runtime_error if file is not a valid morphology file
     * @version 3.0
     */
    BRION_API explicit Morphology(const URI& source);

    /** @return the cell family of that morphology. @version 1.8 */
    BRION_API CellFamily getCellFamily() const;

    /** Get points of morphology, representing x,y,z coordinates + diameter.
     *
     * @return x,y,z coords + diameter of all points of the morphology
     * @version 3.0
     */
    BRION_API Vector4fs& getPoints();
    BRION_API const Vector4fs& getPoints() const;

    /** Get sections of morphology, representing section start index and index
     *  of the parent section.
     *
     * @return First point and parent indices of all the sections of the
     *         morphology.
     * @version 3.0
     */
    BRION_API Vector2is& getSections();
    BRION_API const Vector2is& getSections() const;

    /** Get section types of morphology.
     *
     * @return type of all sections of the morphology
     * @version 3.0
     */
    BRION_API SectionTypes& getSectionTypes();
    BRION_API const SectionTypes& getSectionTypes() const;

    /**
     * @return perimeters of the cross sections for each point of the morphology
     *         in micrometers.
     * @throw std::runtime_error if empty for FAMILY_GLIA
     * @version 3.0
     */
    BRION_API floats& getPerimeters();
    BRION_API const floats& getPerimeters() const;

    /** @internal */
    BRION_API MorphologyVersion getVersion() const;

    /** @internal */
    const MorphologyInitData& getInitData() const;

    servus::Serializable::Data toBinary() const; //!< @internal
                                                 //@}

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
}
#endif
