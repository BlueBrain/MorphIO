#pragma once

#include <morphio/properties.h>
#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio {
/**
 * A class to represent a neuron soma.
 *
 * This class provides functions to query information about the soma of a
 * neuron.
 *
 * Typically the soma is described as the poly-line of the projection
 * of the soma onto a plane, where the plane normal points in the vertical
 * direction in the local coordinate system of the morphology. In other cases
 * the poly-line is not projected onto a plane, but is an approximation of
 * the countour of the soma as seen in an orhogonal projection down the
 * vertical axis (this is basically the same as before, but the vertical
 * coordinate is not 0 for all the points).
 * This class can also be used for both descriptions as well as somas simply
 * approximated as spheres.
 *
 * The coordinates system used by a soma will be in the same as the
 * brain::Morphology from where it comes.
 *
 * @version unstable
 */
struct Soma
{
    Soma() = default;
    Soma(const Soma& soma) = default;

    explicit Soma(const Property::Properties& properties);
    explicit Soma(const Property::PointLevel& point_properties);

    /// Return the  coordinates (x,y,z) of all soma points
    std::vector<Point>& points() noexcept {
        return properties_._points;
    }
    const std::vector<Point>& points() const noexcept {
        return properties_._points;
    }

    /// Return the diameters of all soma points
    std::vector<morphio::floatType>& diameters() noexcept {
        return properties_._diameters;
    }
    const std::vector<morphio::floatType>& diameters() const noexcept {
        return properties_._diameters;
    }

    /// Return the soma type
    SomaType type() const noexcept {
        return soma_type_;
    }

    /// Return the center of gravity of the soma points
    Point center() const;

    /**
     * Return the soma volume\n"
     * Note: the soma volume computation depends on the soma type
     **/
    floatType volume() const;

    /**
     * Return the soma surface\n"
     * Note: the soma surface computation depends on the soma type
     **/
    floatType surface() const;

    /**
     * Return the maximum distance between the center of gravity and any of
     * the soma points
     */
    floatType maxDistance() const;

    /// Return soma properties
    Property::PointLevel& properties() noexcept {
        return properties_;
    }
    const Property::PointLevel& properties() const noexcept {
        return properties_;
    }

  private:

    SomaType soma_type_ = SOMA_UNDEFINED;
    Property::PointLevel properties_;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Soma>& soma);
std::ostream& operator<<(std::ostream& os, const Soma& soma);

}  // namespace morphio
