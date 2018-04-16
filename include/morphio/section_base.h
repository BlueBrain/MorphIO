#pragma once


#include <morphio/iterators.h>
#include <morphio/morphology.h>
#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio
{

/**
   This CRTP (https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
   defines basic methods that every kind of sections (should them be neuronal or mithochondrial)
   must define.

   The CRTP is used here so that the methods of the base class can return object of the derived class.
   Examples:
       T SectionBase::parent()
       std::vector<T> SectionBase::children()
 **/

template <typename T>
class SectionBase
{
public:
    SectionBase(const SectionBase& section);

    const SectionBase& operator=(const SectionBase& section);

    bool operator==(const SectionBase& section) const;
    bool operator!=(const SectionBase& section) const;

    bool isRoot() const;

    /**
     * Return the parent section of this section \if pybind or None if doesn't
     * have any.\else.
     *
     * @throw runtime_error is the section doesn't have a parent.
     * \endif
     */
    T parent() const;

    /**
     * Return a vector with all the direct children of this section.
     * The container will be empty for terminal sections.
     */
    const std::vector<T> children() const;

    /** Return the ID of this section. */
    const uint32_t id() const;

protected:
    SectionBase(uint32_t id, std::shared_ptr<Property::Properties> morphology);
    template <typename Property> const range<const typename Property::Type> get() const;


    uint32_t _id;
    SectionRange _range;
    std::shared_ptr<Property::Properties> _properties;
};

} // namespace morphio

std::ostream& operator<<(std::ostream& os, const morphio::Section& section);
std::ostream& operator<<(std::ostream& os, morphio::range<const morphio::Point> points);

#include "../../src/section_base.tpp"
