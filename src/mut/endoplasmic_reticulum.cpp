#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mut/endoplasmic_reticulum.h>


namespace morphio {
namespace mut {
EndoplasmicReticulum::EndoplasmicReticulum(const std::vector<uint32_t>& sectionIndices,
                                           const std::vector<morphio::floatType>& volumes,
                                           const std::vector<morphio::floatType>& surfaceAreas,
                                           const std::vector<uint32_t>& filamentCounts) {
    _properties._sectionIndices = sectionIndices;
    _properties._volumes = volumes;
    _properties._surfaceAreas = surfaceAreas;
    _properties._filamentCounts = filamentCounts;
}

EndoplasmicReticulum::EndoplasmicReticulum(const EndoplasmicReticulum& endoplasmicReticulum)
    : _properties(endoplasmicReticulum._properties) {}

EndoplasmicReticulum::EndoplasmicReticulum(
    const morphio::EndoplasmicReticulum& endoplasmicReticulum)
    : _properties(endoplasmicReticulum._properties->_endoplasmicReticulumLevel) {}

const std::vector<uint32_t>& EndoplasmicReticulum::sectionIndices() const noexcept {
    return _properties._sectionIndices;
}
std::vector<uint32_t>& EndoplasmicReticulum::sectionIndices() noexcept {
    return _properties._sectionIndices;
}

const std::vector<morphio::floatType>& EndoplasmicReticulum::volumes() const noexcept {
    return _properties._volumes;
}

std::vector<morphio::floatType>& EndoplasmicReticulum::volumes() noexcept {
    return _properties._volumes;
}

const std::vector<morphio::floatType>& EndoplasmicReticulum::surfaceAreas() const noexcept {
    return _properties._surfaceAreas;
}

std::vector<morphio::floatType>& EndoplasmicReticulum::surfaceAreas() noexcept {
    return _properties._surfaceAreas;
}

const std::vector<uint32_t>& EndoplasmicReticulum::filamentCounts() const noexcept {
    return _properties._filamentCounts;
}

std::vector<uint32_t>& EndoplasmicReticulum::filamentCounts() noexcept {
    return _properties._filamentCounts;
}

Property::EndoplasmicReticulumLevel EndoplasmicReticulum::buildReadOnly() const noexcept {
    return _properties;
}

}  // namespace mut
}  // namespace morphio
