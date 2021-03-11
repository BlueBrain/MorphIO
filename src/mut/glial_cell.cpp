#include <morphio/mut/glial_cell.h>
#include <morphio/glial_cell.h>

namespace morphio {
namespace mut {

GlialCell::GlialCell()
    : Morphology() {
    _cellProperties->_cellFamily = CellFamily::GLIA;
}

GlialCell::GlialCell(const std::string& source)
    : Morphology(source) {
    if (_cellProperties->_cellFamily != CellFamily::GLIA)
        throw(RawDataError("File: " + source +
                           " is not a GlialCell file. It should be a H5 file the cell type GLIA."));
}

GlialCell::GlialCell(const morphio::GlialCell& glialCell, unsigned int options)
    : Morphology() {
    _cellProperties->_cellFamily = CellFamily::GLIA;

    for (const morphio::Section& root : glialCell.rootSections()) {
        appendRootSection(root, true);
    }

    for (const morphio::MitoSection& root : glialCell.mitochondria().rootSections()) {
        mitochondria().appendRootSection(root, true);
    }

    applyModifiers(options);
}


}  // namespace mut
}  // namespace morphio
