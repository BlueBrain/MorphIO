/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <morphio/properties.h>  // Property
#include <morphio/soma.h>        // morphio::soma

namespace morphio {
namespace mut {
/** Mutable(editable) morphio::Soma */
class Soma
{
  public:
    Soma() = default;
    Soma(const Soma& soma) = default;

    explicit Soma(const Property::PointLevel& pointProperties);
    explicit Soma(const morphio::Soma& soma);

    /// Return the coordinates (x,y,z) of all soma points
    std::vector<Point>& points() noexcept {
        return point_properties_._points;
    }
    const std::vector<Point>& points() const noexcept {
        return point_properties_._points;
    }

    /// Return the diameters of all soma points
    std::vector<morphio::floatType>& diameters() noexcept {
        return point_properties_._diameters;
    }
    const std::vector<morphio::floatType>& diameters() const noexcept {
        return point_properties_._diameters;
    }

    /// Return the soma type
    const SomaType& type() const noexcept {
        return soma_type_;
    }

    /// Return the soma type
    SomaType& type() noexcept {
        return soma_type_;
    }

    /**
     * Return the center of gravity of the soma points
     **/
    Point center() const;

    /**
       Return the soma surface
       Note: the soma surface computation depends on the soma type
    **/
    floatType surface() const;

    /**
     * Return the maximum distance between the center of gravity and any of
     * the soma points
     */
    floatType maxDistance() const;

    Property::PointLevel& properties() noexcept {
        return point_properties_;
    }
    const Property::PointLevel& properties() const noexcept {
        return point_properties_;
    }

  private:
    friend class Morphology;

    SomaType soma_type_ = SOMA_UNDEFINED;
    Property::PointLevel point_properties_;
};

}  // namespace mut
}  // namespace morphio
