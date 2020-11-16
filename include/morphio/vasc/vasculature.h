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
    Section section(const uint32_t& id) const;

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
     * graph iterators
     **/
    graph_iterator begin() const;
    graph_iterator end() const;

  private:
    std::shared_ptr<property::Properties> _properties;

    template <typename Property>
    inline const std::vector<typename Property::Type>& get() const noexcept;
};

template <typename Property>
inline const std::vector<typename Property::Type>& Vasculature::get() const noexcept {
    return _properties->get<Property>();
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
