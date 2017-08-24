/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
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

#ifndef BRAIN_NEURON_MORPHOLOGY
#define BRAIN_NEURON_MORPHOLOGY

#include <brain/api.h>
#include <brain/neuron/types.h>
#include <brain/types.h>

#include <boost/noncopyable.hpp>
#include <servus/serializable.h>

namespace brain
{
namespace neuron
{
/**
 * Wrapper around brion::Morphology with higher level functions.
 *
 * This class provides methods to facilitate some queries about morphologies
 * in the context of circuits.
 * Morphologies can be loaded with a transformation applied to its points,
 * which is useful for operating in global circuit coordinates.
 * The transformation is applied at construction so it cannot be modified or
 * reverted.
 *
 * Access to the raw data fields is still provided by getter functions.
 *
 * @version unstable
 */
class Morphology : public boost::noncopyable
{
public:
    class Impl;                            //!< @internal
    using ImplPtr = std::shared_ptr<Impl>; //!< @internal

    /**
     * Create a morphology from a URI and load all the data.
     *
     * @param source URI of the morphology data source.
     * @throw runtime_error if an inconsistency is detected in the input file.
     */
    BRAIN_API explicit Morphology(const URI& source);

    /**
     * Create a morphology from a URI, load all the data and transform
     * the points.
     *
     * @param source URI of the morphology data source.
     * @param transform the transformation matrix to apply to the points.
     *        Radii will not be affected by this transformation.
     * @throw runtime_error if an inconsistency is detected in the input file.
     */
    BRAIN_API Morphology(const URI& source, const Matrix4f& transform);

    /**
     * Create a morphology from a brion::Morphology and load all the data.
     *
     * @param morphology the brion::Morphology to load from.
     * @throw runtime_error if an inconsistency is detected in the input file.
     */
    BRAIN_API explicit Morphology(brion::ConstMorphologyPtr morphology);

    /**
     * Create a morphology from a brion::Morphology, load all the data
     * and transform the points. The given morphology is modified.
     *
     * @param morphology the brion::Morphology to load from.
     * @param transform the transformation matrix to apply to the points.
     *        Radii will not be affected by this transformation.
     * @throw runtime_error if an inconsistency is detected in the input file.
     */
    BRAIN_API Morphology(brion::MorphologyPtr morphology,
                         const Matrix4f& transform);

    BRAIN_API ~Morphology();

    /** @sa brion::Morphology::readPoints */
    BRAIN_API const Vector4fs& getPoints() const;

    /** @sa brion::Morphology::readSections */
    BRAIN_API const Vector2is& getSections() const;

    /** @sa brion::Morphology::readSectionTypes
        This type is not brain::SectionTypes because brion::SectionType
        is not convertible to brain::neuron::SectionType. */
    BRAIN_API const SectionTypes& getSectionTypes() const;

    /** Return the list of ids for the given section types. */
    BRAIN_API uint32_ts getSectionIDs(const SectionTypes& types) const;

    /**
     * Return the sections which have the given section type.
     * If type is SectionType::Soma an empty list is returned.
     */
    BRAIN_API Sections getSections(SectionType type) const;

    /**
     * Return the sections which have any of the given section types.
     * No sections are returned for the type SectionType::Soma.
     */
    BRAIN_API Sections getSections(const SectionTypes& types) const;

    /**
     * Return the Section with the given id.
     *
     * @throw runtime_error if the id is out of range or the given id refers to
     * a soma section.
     */
    BRAIN_API Section getSection(const uint32_t& id) const;

    /** Return the object with the information about the neuron soma */
    BRAIN_API Soma getSoma() const;

    /** Return \if pybind a 4x4 numpy array with \endif
     *  the transformation that was passed to the constructor or the
     *  identity matrix is no transformation was given.
     */
    BRAIN_API const Matrix4f& getTransformation() const;

private:
    friend class brain::Circuit;
    Morphology(const void* data, const size_t size);
    servus::Serializable::Data toBinary() const;

    ImplPtr _impl;
};
}
}
#endif
