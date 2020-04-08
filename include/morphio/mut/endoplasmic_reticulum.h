#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio {
namespace mut {
/**
 * The entry-point class to access endoplasmic reticulum data
 *
 * Spec https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/h5v1.html
 **/
class EndoplasmicReticulum
{
  public:
    EndoplasmicReticulum() = default;
    EndoplasmicReticulum(const std::vector<uint32_t>& sectionIndices,
                         const std::vector<morphio::floatType>& volumes,
                         const std::vector<morphio::floatType>& surfaceAreas,
                         const std::vector<uint32_t>& filamentCounts);
    EndoplasmicReticulum(const EndoplasmicReticulum& endoplasmicReticulum);
    EndoplasmicReticulum(const morphio::EndoplasmicReticulum& endoplasmicReticulum);


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
    morphio::Property::EndoplasmicReticulumLevel _properties;
    EndoplasmicReticulum(const morphio::Property::EndoplasmicReticulumLevel&);
};
}  // namespace mut
}  // namespace morphio
