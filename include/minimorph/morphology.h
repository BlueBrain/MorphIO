#pragma once

#include <memory>  //std::unique_ptr

#include <minimorph/api.h>
#include <minimorph/types.h>
#include <minimorph/properties.h>
#include <minimorph/soma.h>

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

    const Points getPoints() const;
    const floats getDiameters() const;
    const floats getPerimeters() const;
    const SectionTypes getSectionTypes() const;
    const CellFamily getCellFamily() const;

    /** @internal */
    BRAIN_API MorphologyVersion getVersion() const;

private:
    PropertiesPtr _properties;

    template <typename Property> typename Property::Type& get();
    template <typename Property> const typename Property::Type& get() const;
};

}
