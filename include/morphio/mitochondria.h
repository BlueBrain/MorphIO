/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <memory>
#include <vector>

#include <morphio/morphology.h>  // Morphology
#include <morphio/properties.h>  // Property

namespace morphio {
/**
 * The entry-point class to access mitochondrial data
 *
 * By design, it is the equivalent of the Morphology class but at the
 * mitochondrial level. As the Morphology class, it implements a section accessor
 * and a root section accessor returning views on the Properties object for the
 * queried mitochondrial section.
 **/
class Mitochondria
{
  public:
    /// Return the Section with the given id.
    MitoSection section(uint32_t id) const;

    /// Return a vector of all root sections
    std::vector<MitoSection> rootSections() const;

    /** Return a vector containing all section objects
     *
     * Notes:
     * Soma is not included
     **/
    std::vector<MitoSection> sections() const;

  private:
    explicit Mitochondria(const std::shared_ptr<Property::Properties>& properties)
        : properties_(properties) {}
    std::shared_ptr<Property::Properties> properties_;

    friend class Morphology;
};
}  // namespace morphio
