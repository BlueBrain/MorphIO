#pragma once

#include <memory> //std::unique_ptr

#include <morphio/api.h>
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
    BRAIN_API ~Morphology();

    BRAIN_API Morphology(const builder::Morphology&);
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

    BRAIN_API const Soma soma() const;
    BRAIN_API const std::vector<Section> rootSections() const;
    BRAIN_API const std::vector<Section> sections() const;

    /**
     * Return the Section with the given id.
     *
     * @throw runtime_error if the id is out of range or the given id refers to
     * a soma section.
     */
    BRAIN_API const Section section(const uint32_t& id) const;

    BRAIN_API const Points& points() const;
    BRAIN_API const std::vector<float>& diameters() const;
    BRAIN_API const std::vector<float>& perimeters() const;
    BRAIN_API const std::vector<SectionType>& sectionTypes() const;
    BRAIN_API const CellFamily& cellFamily() const;
    BRAIN_API const MorphologyVersion& version() const;

private:
    PropertiesPtr _properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;
};
}
