#pragma once

#include <memory> //std::unique_ptr

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
enum SomaClasses
{
    SOMA_CONTOUR,
    SOMA_CYLINDER
};

/** Read access a Morphology file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
class Morphology
{
public:
    virtual ~Morphology();

    Morphology& operator=(const Morphology&);
    Morphology(Morphology&&);
    Morphology& operator=(Morphology&&);

    /**
       Equality operator:
           Are considered equal, 2 morphologies with the same:
               - point vector
               - diameter vector
               - perimeter vector
               - cell family
               - section types
               - topology (children/parent relationship)

           Note: the soma types are NOT required to be equal
     **/
    bool operator==(const Morphology& other) const;
    bool operator!=(const Morphology& other) const;

    /** @name Read API */
    //@{
    /** Open the given source to a morphology file and parse it.

        options is the modifier flags to be applied. All flags are defined in
       their enum: morphio::enum::Option and can be composed.

        Example:
            Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
     */
    Morphology(const URI& source, unsigned int options = NO_MODIFIER);
    Morphology(mut::Morphology);

    /**
     * Return the soma object
     **/
    const Soma soma() const;

    /**
     * Return the mitochondria object
     **/
    const Mitochondria mitochondria() const;

    /**
     * Return the annotation object
     **/
    const std::vector<Property::Annotation> annotations() const;

    /**
     * Return a vector of all root sections
     * (sections whose parent ID are -1)
     **/
    const std::vector<Section> rootSections() const;

    /**
     * Return a vector containing all section objects.
     *
     * The first section of the vector is always the soma section.
     **/
    const std::vector<Section> sections() const;

    /**
     * Return the Section with the given id.
     *
     * @throw RawDataError if the id is out of range
     */
    const Section section(const uint32_t& id) const;

    /**
     * Return a vector with all points from all sections
     * (soma points are not included)
     **/
    const Points& points() const;

    /**
     * Return a vector with all diameters from all sections
     * (soma points are not included)
     **/
    const std::vector<float>& diameters() const;

    /**
     * Return a vector with all perimeters from all sections
     **/
    const std::vector<float>& perimeters() const;

    /**
     * Return a vector with the section type of every section
     **/
    const std::vector<SectionType>& sectionTypes() const;

    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;

    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

    /**
     * Return the soma type
     **/
    const SomaType& somaType() const;

    /**
     * Return the cell family (neuron or glia)
     **/
    const CellFamily& cellFamily() const;

    /**
     * Return the version
     **/
    const MorphologyVersion& version() const;

private:
    friend class mut::Morphology;

    std::shared_ptr<Property::Properties> _properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;
};
} // namespace morphio
