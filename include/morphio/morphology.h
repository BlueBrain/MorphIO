/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <highfive/H5Group.hpp>
#include <morphio/properties.h>
#include <morphio/section_iterators.hpp>
#include <morphio/types.h>

namespace morphio {

/** Morphology breadth iterator */
using breadth_iterator = breadth_iterator_t<Section, Morphology>;
/** Morphology depth iterator */
using depth_iterator = depth_iterator_t<Section, Morphology>;

/** Class that gives read access to a Morphology file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class Morphology
{
  public:
    virtual ~Morphology() = default;

    Morphology(const Morphology&) noexcept = default;
    Morphology& operator=(const Morphology&) noexcept = default;
    Morphology(Morphology&&) noexcept = default;
    Morphology& operator=(Morphology&&) noexcept = default;

    /** @name Read API */

    /** Open the given source to a morphology file and parse it.

       \param source path to a source file.
       \param options is the modifier flags to be applied. All flags are defined in
         their corresponding morphio.enums.Option and can be composed.

        Example:
            Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
     */
    explicit Morphology(const std::string& path, unsigned int options = NO_MODIFIER);

    /** Constructor from an already parsed file */
    explicit Morphology(const HighFive::Group& group, unsigned int options = NO_MODIFIER);

    /** Constructor from an instance of morphio::mut::Morphology */
    explicit Morphology(const mut::Morphology&);

    /** Load a morphology from a string */
    explicit Morphology(const std::string& contents,
                        const std::string& extension,
                        unsigned int options = NO_MODIFIER);

    /** Return the soma object */
    Soma soma() const;

    /** Return the mitochondria object */
    Mitochondria mitochondria() const;

    /** Return the endoplasmic reticulum object */
    EndoplasmicReticulum endoplasmicReticulum() const;

    /** Return the annotation object */
    const std::vector<Property::Annotation>& annotations() const;

    /** Return the markers */
    const std::vector<Property::Marker>& markers() const;

    /**
     * Return a vector of all root sections
     * (sections whose parent ID are -1)
     **/
    std::vector<Section> rootSections() const;

    /** Return a vector containing all section objects
     *
     * Notes:
     * Soma is not included
     **/
    std::vector<Section> sections() const;

    /**
     * Return the Section with the given id.
     *
     * @throw RawDataError if the id is out of range
     */
    Section section(uint32_t id) const;

    /**
     * Return a vector with all points from all sections
     * (soma points are not included)
     **/
    const Points& points() const noexcept;

    /**
     * Returns a list with offsets to access data of a specific section in the points
     * and diameters arrays.
     *
     * Example: accessing diameters of n'th section will be located in the Morphology::diameters
     * array from diameters[sectionOffsets(n)] to diameters[sectionOffsets(n+1)-1]
     *
     * Note: for convenience, the last point of this array is the points() array size
     * so that the above example works also for the last section.
     **/
    std::vector<uint32_t> sectionOffsets() const;

    /**
     * Return a vector with all diameters from all sections
     * (soma points are not included)
     **/
    const std::vector<morphio::floatType>& diameters() const;

    /** Return a vector with all perimeters from all sections */
    const std::vector<morphio::floatType>& perimeters() const;

    /** Return a vector with the section type of every section */
    const std::vector<SectionType>& sectionTypes() const;

    /**
     * Return the graph connectivity of the morphology where each section
     * is seen as a node
     * Note: -1 is the soma node
     **/
    const std::map<int, std::vector<unsigned int>>& connectivity() const;

    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    depth_iterator depth_begin() const;
    /** depth end iterator */
    depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    breadth_iterator breadth_begin() const;
    /** breadth end iterator */
    breadth_iterator breadth_end() const;

    /** Return the soma type */
    const SomaType& somaType() const;

    /** Return the cell family (neuron or glia) */
    const CellFamily& cellFamily() const;

    /** Return the version */
    const MorphologyVersion& version() const;

  protected:
    friend class mut::Morphology;
    Morphology(const Property::Properties& properties, unsigned int options);

    std::shared_ptr<Property::Properties> properties_;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;
};
}  // namespace morphio
