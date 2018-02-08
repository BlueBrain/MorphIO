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

#ifndef BRAIN_MORPHOLOGY
#define BRAIN_MORPHOLOGY

#include "api.h"
#include <types.h>

#include <memory>  //std::unique_ptr

#include <properties.hpp>
#include <soma.h>

namespace minimorph
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
    BRAIN_API ~Morphology();

    BRAIN_API Morphology& operator=(const Morphology&);
    BRAIN_API Morphology(Morphology&&);
    BRAIN_API Morphology& operator=(Morphology&&);


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
    BRAIN_API explicit Morphology(const URI& source);

    Soma getSoma() const;

    Sections getSections();

    /**
     * Return the Section with the given id.
     *
     * @throw runtime_error if the id is out of range or the given id refers to
     * a soma section.
     */
    BRAIN_API Section getSection(const uint32_t& id) const;


    BRAIN_API template <typename Property> typename Property::Type& get();
    BRAIN_API template <typename Property> const typename Property::Type& get() const;

    /** @internal */
    BRAIN_API MorphologyVersion getVersion() const;

private:
    PropertiesPtr _properties;
};

}
#endif
