#include <morphio/endoplasmic_reticulum.h>
#include <morphio/properties.h>

namespace morphio {
const std::vector<uint32_t>& EndoplasmicReticulum::sectionIndices() const {
    return _properties->_endoplasmicReticulumLevel._sectionIndices;
}

const std::vector<float>& EndoplasmicReticulum::volumes() const {
    return _properties->_endoplasmicReticulumLevel._volumes;
}

const std::vector<float>& EndoplasmicReticulum::surfaceAreas() const {
    return _properties->_endoplasmicReticulumLevel._surfaceAreas;
}

const std::vector<uint32_t>& EndoplasmicReticulum::filamentCounts() const {
    return _properties->_endoplasmicReticulumLevel._filamentCounts;
}

}  // namespace morphio
