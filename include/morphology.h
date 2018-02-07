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

#include "api.h"
#include <types.h>

#include <memory>  //std::unique_ptr


namespace minimorph
{
/** Read access a Morphology file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */


// struct IDProperty {
//     static const auto value = 0;
// };

struct PointProperty {
    typedef std::array<float, 3> Type;
};


class Morphology
{
public:
    /** Close morphology file. @version 1.0 */
    BRION_API ~Morphology();

#if 0
    BRION_API Morphology(const void* data, size_t size);
    BRION_API Morphology(const Morphology&);
#endif
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

    template <typename Property> std::vector<typename Property::Type> get();



    /** @internal */
    BRION_API MorphologyVersion getVersion() const;

    /** @internal */
    const MorphologyInitData& getInitData() const;
    class Impl;

private:
    std::unique_ptr<Impl> _impl;
};


template <> std::vector<typename PointProperty::Type> Morphology::get<PointProperty>();
}
#endif
