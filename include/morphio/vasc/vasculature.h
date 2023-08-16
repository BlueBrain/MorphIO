/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>  // std::shared_ptr
#include <string>  // std::string
#include <vector>  // std::vector

#include <morphio/types.h>
#include <morphio/vasc/iterators.hpp>
#include <morphio/vasc/properties.h>

namespace morphio {
namespace vasculature {

using graph_iterator = graph_iterator_t<Section, Vasculature>;

/**
 * The entry-point class to access vasculature(blood) data
 *
 * By design, it is the equivalent of the Morphology class but at the
 * vasculature level. As the Morphology class, it implements a section accessor
 * and a root section accessor returning views on the Properties object for the
 * queried vasculature section.
 **/
class Vasculature
{
  public:
    /** @name Read API */
    /** Open the given source to a vasculature file and parse it.
     */
    explicit Vasculature(const std::string& source);

    Vasculature(Vasculature&&) = default;
    virtual ~Vasculature() = default;

    Vasculature& operator=(const Vasculature&) = default;
    Vasculature& operator=(Vasculature&&) = default;

    /**
     * Return a vector containing all section objects.
     **/
    std::vector<Section> sections() const;

    /**
     * Return the Section with the given id.
     *
     * @throw RawDataError if the id is out of range
     */
    Section section(uint32_t id) const;

    /**
     * Returns a list with offsets to access data of a specific section in the points
     * and diameters arrays.
     p
     * Example: accessing diameters of n'th section will be located in the Vasculature::diameters
     * array from diameters[sectionOffsets(n)] to diameters[sectionOffsets(n+1)-1]
     *
     * Note: for convenience, the last point of this array is the points() array size
     * so that the above example works also for the last section.
     */
    const std::vector<uint32_t> sectionOffsets() const noexcept;

    /**
     * Return a vector with all points from all sections
     **/
    inline const Points& points() const noexcept;

    /**
     * Return a vector with all diameters from all sections
     **/
    inline const std::vector<morphio::floatType>& diameters() const noexcept;

    /**
     * Return a vector with the section type of every section
     **/
    inline const std::vector<property::SectionType::Type>& sectionTypes() const noexcept;

    /**
     * Return a vector with all the connections between sections
     **/
    const std::vector<morphio::vasculature::property::Connection::Type>& sectionConnectivity() const
        noexcept;


    /** graph iterator pointing to the begin */
    graph_iterator begin() const;
    /** graph iterator pointing to the end */
    graph_iterator end() const;

  private:
    std::shared_ptr<property::Properties> properties_;

    template <typename Property>
    inline const std::vector<typename Property::Type>& get() const noexcept;
};

template <typename Property>
inline const std::vector<typename Property::Type>& Vasculature::get() const noexcept {
    return properties_->get<Property>();
}

inline const Points& Vasculature::points() const noexcept {
    return get<property::Point>();
}

inline const std::vector<morphio::floatType>& Vasculature::diameters() const noexcept {
    return get<property::Diameter>();
}

inline const std::vector<property::SectionType::Type>& Vasculature::sectionTypes() const noexcept {
    return get<property::SectionType>();
}

}  // namespace vasculature
}  // namespace morphio
