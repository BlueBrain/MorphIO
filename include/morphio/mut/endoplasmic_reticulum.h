/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
/** Mutable(editable) morphio::EndoplasmicReticulum */
class EndoplasmicReticulum
{
  public:
    EndoplasmicReticulum() = default;
    EndoplasmicReticulum(const EndoplasmicReticulum& endoplasmicReticulum) = default;

    EndoplasmicReticulum(const std::vector<uint32_t>& section_indices,
                         const std::vector<morphio::floatType>& volumes,
                         const std::vector<morphio::floatType>& surface_areas,
                         const std::vector<uint32_t>& filament_counts);

    explicit EndoplasmicReticulum(const morphio::EndoplasmicReticulum& endoplasmic_reticulum);


    /**
       Returns the list of neuronal section indices
    **/
    const std::vector<uint32_t>& sectionIndices() const noexcept;
    std::vector<uint32_t>& sectionIndices() noexcept;

    /**
       Returns the volumes for each neuronal section
    **/
    const std::vector<morphio::floatType>& volumes() const noexcept;
    std::vector<morphio::floatType>& volumes() noexcept;

    /**
       Returns the surface areas for each neuronal section
    **/
    const std::vector<morphio::floatType>& surfaceAreas() const noexcept;
    std::vector<morphio::floatType>& surfaceAreas() noexcept;

    /**
       Returns the number of filaments for each neuronal section
    **/
    const std::vector<uint32_t>& filamentCounts() const noexcept;
    std::vector<uint32_t>& filamentCounts() noexcept;

    /**
       Returns the data structure that stores ER data
       This data structure is used to create the immutable object
    **/
    Property::EndoplasmicReticulumLevel buildReadOnly() const noexcept;

  private:
    morphio::Property::EndoplasmicReticulumLevel properties_;
    explicit EndoplasmicReticulum(const morphio::Property::EndoplasmicReticulumLevel&);
};
}  // namespace mut
}  // namespace morphio
