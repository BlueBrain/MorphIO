#pragma once

#include <memory> //std::unique_ptr

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio
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
    ~Morphology();


    Morphology& operator=(const Morphology&);
    Morphology(Morphology&&);
    Morphology& operator=(Morphology&&);

    bool operator==(const Morphology& other) const;

    /** @name Read API */
    //@{
    /** Open the given source to a morphology file and parse it.

        options is the modifier flags to be applied. All flags are defined in their enum:
        morphio::enum::Option and can be composed.

        Example:
            Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
     */
    explicit Morphology(const URI& source, unsigned int options);
    Morphology(const mut::Morphology&);

    const Soma soma() const;
    const std::vector<Section> rootSections() const;

    /**
     * Return a vector containing all section objects.
     *
     * The first section of the vector is the soma section.
     **/
    const std::vector<Section> sections() const;

    /**
     * Return the Section with the given id.
     *
     * @throw runtime_error if the id is out of range or the given id refers to
     * a soma section.
     */
    const Section section(const uint32_t& id) const;

    const Points& points() const;
    const std::vector<float>& diameters() const;
    const std::vector<float>& perimeters() const;
    const std::vector<SectionType>& sectionTypes() const;
    const SomaType& somaType() const;
    const CellFamily& cellFamily() const;
    const MorphologyVersion& version() const;

private:
    std::shared_ptr<Property::Properties> _properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;
};
}
