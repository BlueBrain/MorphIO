#pragma once

#include <memory>
#include <morphio/types.h>
#include <morphio/vasc/properties.h>

namespace morphio {
namespace vasculature {
class Vasculature
{
public:
    virtual ~Vasculature();

    Vasculature& operator=(const Vasculature&);
    Vasculature(Vasculature&&);
    Vasculature& operator=(Vasculature&&);

    /** @name Read API */
    /** Open the given source to a vasculature file and parse it.
     */
    explicit Vasculature(const std::string& source);

    /**
     * Return a vector containing all section objects.
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
     **/
    const Points& points() const;

    /**
     * Return a vector with all diameters from all sections
     **/
    const std::vector<float>& diameters() const;

    /**
     * Return a vector with the section type of every section
     **/
    const std::vector<property::SectionType::Type>& sectionTypes() const;

    /**
     * graph iterators
    **/
    graph_iterator begin() const;
    graph_iterator end() const;

private:
    std::shared_ptr<property::Properties> _properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;
};
} // namespace vasculature
} // namespace morphio
