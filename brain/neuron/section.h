/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
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

#ifndef BRAIN_NEURON_SECTION
#define BRAIN_NEURON_SECTION

#include <brain/api.h>
#include <brain/types.h>

#include <brain/neuron/morphology.h>

namespace brain
{
namespace neuron
{

/**
 * A class to represent a morphological section.
 *
 * A Section is an unbranched piece of a morphological skeleton.
 * This class provides functions to query information about the sample points
 * that compose the section and functions to obtain the parent and children
 * sections.
 *
 * The cell soma is also considered a section, but some functions have
 * special meaning for it.
 *
 * Sections cannot be directly created, but are returned by several
 * brain::Morphology and brain::Section methods.
 *
 * This is a lightweight object with STL container style thread safety.
 * It is also safe to use a section after the morphology from where it comes
 * has been deallocated. The morphological data will be kept as long as there
 * is a Section referring to it.
 */
class Section
{
public:
    friend class Morphology;
    friend class Soma;

    BRAIN_API Section( const Section& section );

    BRAIN_API ~Section();

    BRAIN_API Section& operator=( const Section& section );

    BRAIN_API bool operator==( const Section& section ) const;
    BRAIN_API bool operator!=( const Section& section ) const;

    /** Return the ID of this section. */
    BRAIN_API uint32_t getID() const;

    /** Return the morphological type of this section (dendrite, axon, ...). */
    BRAIN_API SectionType getType() const;

    /**
     * Return the total length of this section in microns.
     *
     * If this section is a soma section the length is ill-defined and this
     * function will return 0.
     */
    BRAIN_API float getLength() const;

    /**
     * Return the list of all point samples that define this section.
     *
     * If this sections is a soma section return the list of points of the
     * soma profile poly-line.
     *
     * @return A list of point positions with diameter. For a section consisting
     *         of n segments, this list will have n + 1 points.
     */
    BRAIN_API Vector4fs getSamples() const;

    /**
     * Return a list of points sampling this section at discrete locations.
     *
     * If the section is a soma section this function will return the soma
     * position for all sampling positions. The soma position is assumed to
     * be (0, 0, 0) unless the origin morphology has been transformed.
     *
     * @param points Normalized positions of the sample points along the
     *        section. Values will be clampled to [0, 1] before sampling.
     * @return The section sampled at the given relative positions.
     */
    BRAIN_API Vector4fs getSamples( const floats& points ) const;

    /**
     * Return the absolute distance from the start of the section to the soma.
     */
    BRAIN_API float getDistanceToSoma() const;

    /**
     * Return the absolute distances to the soma in microns for all sample
     * positions.
     *
     * @return A list of distances. For a section consisting
     *         of n segments, this list will have n + 1 values. The section
     *         length is equal to the difference between the first and last
     *         values of the list.
     */
    BRAIN_API floats getSampleDistancesToSoma() const;

    /** Return true if this section has a parent section, false otherwise. */
    BRAIN_API bool hasParent() const;

    /**
     * Return the parent section of this section \if pybind or None if doesn't
     * have any.\else.
     *
     * @throw runtime_error is the section doesn't have a parent.
     * \endif
     */
    BRAIN_API Section getParent() const;

    /**
     * Return a vector with all the direct children of this section.
     * The container will be empty for terminal sections.
     */
    BRAIN_API Sections getChildren() const;

protected:
    BRAIN_API Section( uint32_t id, Morphology::Impl* morphology );

private:
    uint32_t _id;
    Morphology::Impl* _morphology;
};

}
}
#endif
