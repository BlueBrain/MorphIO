/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <fstream>
#include <iomanip>  // std::fixed, std::setw, std::setprecision

#include <morphio/errorMessages.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/version.h>

#include "../shared_utils.hpp"
#include "writer_utils.h"

namespace {
void write_asc_points(std::ofstream& myfile,
                      const morphio::Points& points,
                      const std::vector<morphio::floatType>& diameters,
                      size_t indentLevel) {
    for (unsigned int i = 0; i < points.size(); ++i) {
        myfile << std::fixed << std::setprecision(morphio::FLOAT_PRECISION_PRINT)
               << std::string(indentLevel, ' ') << '(' << points[i][0] << ' ' << points[i][1] << ' '
               << points[i][2] << ' ' << diameters[i] << ")\n";
    }
}

void write_asc_section(std::ofstream& myfile,
                       const std::shared_ptr<morphio::mut::Section>& section,
                       size_t indentLevel) {
    const std::string indent(indentLevel, ' ');
    write_asc_points(myfile, section->points(), section->diameters(), indentLevel);

    if (!section->children().empty()) {
        auto children = section->children();
        size_t nChildren = children.size();
        for (unsigned int i = 0; i < nChildren; ++i) {
            myfile << indent << (i == 0 ? "(\n" : "|\n");
            write_asc_section(myfile, children[i], indentLevel + 2);
        }
        myfile << indent << ")\n";
    }
}
}  // namespace

namespace morphio {
namespace mut {
namespace writer {

void asc(const Morphology& morph, const std::string& filename) {
    if (details::emptyMorphology(morph)) {
        return;
    }

    details::validateContourSoma(morph);
    details::checkSomaHasSameNumberPointsDiameters(*morph.soma());
    details::validateHasNoMitochondria(morph);
    details::validateHasNoPerimeterData(morph);

    std::ofstream myfile(filename);

    const std::shared_ptr<Soma>& soma = morph.soma();
    if (!soma->points().empty()) {
        myfile << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
        write_asc_points(myfile, soma->points(), soma->diameters(), 2);
        myfile << ")\n\n";
    }

    for (const std::shared_ptr<Section>& section : morph.rootSections()) {
        const auto type = section->type();
        if (type == SECTION_AXON) {
            myfile << "( (Color Cyan)\n  (Axon)\n";
        } else if (type == SECTION_DENDRITE) {
            myfile << "( (Color Red)\n  (Dendrite)\n";
        } else if (type == SECTION_APICAL_DENDRITE) {
            myfile << "( (Color Red)\n  (Apical)\n";
        } else {
            throw WriterError(readers::ErrorMessages().ERROR_UNSUPPORTED_SECTION_TYPE(type));
        }
        write_asc_section(myfile, section, 2);
        myfile << ")\n\n";
    }

    myfile << "; " << details::version_string() << '\n';
}
}  // end namespace writer
}  // end namespace mut
}  // end namespace morphio
