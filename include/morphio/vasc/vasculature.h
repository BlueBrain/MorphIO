#pragma once

#include <memory> // std::shared_ptr
#include <string> // std::string
#include <vector> // std::vector

#include <morphio/types.h>
#include <morphio/vasc/iterators.hpp>
#include <morphio/vasc/properties.h>

namespace morphio {
namespace vasculature {

using graph_iterator = graph_iterator_t<Section, Vasculature>;

class Vasculature
{
public:
    /** @name Read API */
    /** Open the given source to a vasculature file and parse it.
     */
    explicit Vasculature(const std::string& source);

    Vasculature(Vasculature&&) = default;
    virtual ~Vasculature() {}

    Vasculature& operator=(const Vasculature&) = default;
    Vasculature& operator=(Vasculature&&) = default;

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
