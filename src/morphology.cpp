#include <cassert>
#include <fstream>
#include <memory>
#include <streambuf>
#include <sstream>
#include <string>

#include <morphio/morphology.h>

#include <morphio/endoplasmic_reticulum.h>
#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/shared_utils.tpp>
#include <morphio/soma.h>
#include <morphio/tools.h>


#include "readers/morphologyASC.h"
#include "readers/morphologyHDF5.h"
#include "readers/morphologySWC.h"


#include <iostream>

namespace morphio {


Morphology::Morphology(const Property::Properties& properties, unsigned int options)
    : TTree<Node<CellFamily::NEURON>, Morphology, morphio::mut::Morphology>(properties, options) {
    init();
}

Morphology::Morphology(const std::string& source, unsigned int options)
    : TTree<Node<CellFamily::NEURON>, Morphology, morphio::mut::Morphology>(source, options) {
    init();
}

Morphology::Morphology(const HighFive::Group& group, unsigned int options)
    : TTree<Node<CellFamily::NEURON>, Morphology, morphio::mut::Morphology>(group, options) {
    init();
}

Morphology::Morphology(morphio::mut::Morphology morphology)
    : TTree<Node<CellFamily::NEURON>, Morphology, morphio::mut::Morphology>(morphology) {
    init();
}

void Morphology::init() {
    if (_properties->_cellLevel.fileFormat() != "swc")
        _properties->_cellLevel._somaType = getSomaType(soma().points().size());
}


Soma Morphology::soma() const {
    return Soma(_properties);
}

Mitochondria Morphology::mitochondria() const {
    return Mitochondria(_properties);
}

const EndoplasmicReticulum Morphology::endoplasmicReticulum() const {
    return EndoplasmicReticulum(_properties);
}

const std::vector<Property::Annotation>& Morphology::annotations() const {
    return _properties->_cellLevel._annotations;
}

const std::vector<Property::Marker>& Morphology::markers() const {
    return _properties->_cellLevel._markers;
}

const SomaType& Morphology::somaType() const {
    return _properties->somaType();
}

// Explicit instantiation
template class TTree<Node<CellFamily::NEURON>, Morphology, mut::Morphology>;

}  // namespace morphio
