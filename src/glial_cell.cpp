#include <cassert>
#include <fstream>
#include <memory>
#include <streambuf>
#include <sstream>
#include <string>

#include <morphio/glial_cell.h>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/mut/glial_cell.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/shared_utils.tpp>
#include <morphio/soma.h>
#include <morphio/tools.h>


#include "readers/morphologyASC.h"
#include "readers/morphologyHDF5.h"
#include "readers/morphologySWC.h"

namespace morphio {



GlialCell::GlialCell(const Property::Properties& properties, unsigned int options)
    : TTree<Node<CellFamily::GLIA>, GlialCell, morphio::mut::GlialCell>(properties, options) {
    init();
}

GlialCell::GlialCell(const std::string& source, unsigned int options)
    : TTree<Node<CellFamily::GLIA>, GlialCell, morphio::mut::GlialCell>(source, options) {
    /*
    if (_properties->_cellLevel._cellFamily != CellFamily::GLIA)
        throw(RawDataError("File: " + source +
                           " is not a GlialCell file. It should be a H5 file the cell type GLIA."));
    */
    init();
}

GlialCell::GlialCell(const HighFive::Group& group, unsigned int options)
    : TTree<Node<CellFamily::GLIA>, GlialCell, morphio::mut::GlialCell>(group, options) {
    init();
}

GlialCell::GlialCell(morphio::mut::GlialCell glialCell)
    : TTree<Node<CellFamily::GLIA>, GlialCell, morphio::mut::GlialCell>(glialCell) {
    init();
}

void GlialCell::init() {
    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());
}

Soma GlialCell::soma() const {
    return Soma(_properties);
}

Mitochondria GlialCell::mitochondria() const {
    return Mitochondria(_properties);
}

const EndoplasmicReticulum GlialCell::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

const std::vector<Property::Annotation>& GlialCell::annotations() const {
    return _properties->_cellLevel._annotations;
}

const std::vector<Property::Marker>& GlialCell::markers() const {
    return _properties->_cellLevel._markers;
}

const SomaType& GlialCell::somaType() const {
    return _properties->somaType();
}

// Explicit instantiation
template class TTree<Node<CellFamily::GLIA>, GlialCell, mut::GlialCell>;

}  // namespace morphio
