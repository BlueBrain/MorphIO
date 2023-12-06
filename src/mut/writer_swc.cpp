/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <cassert>
#include <fstream>
#include <iomanip>  // std::fixed, std::setw, std::setprecision

#include <morphio/errorMessages.h>
#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/vector_types.h>
#include <morphio/version.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Object.hpp>

#include "../shared_utils.hpp"
#include "writer_utils.h"

namespace {
void writeLine(std::ofstream& myfile,
               int id,
               int parentId,
               morphio::SectionType type,
               const morphio::Point& point,
               morphio::floatType diameter) {
    using std::setw;
    const size_t width = 12;

    myfile << std::to_string(id) << setw(width) << std::to_string(type) << ' ' << setw(width);
    myfile << std::fixed
#if !defined(MORPHIO_USE_DOUBLE)
           << std::setprecision(morphio::FLOAT_PRECISION_PRINT)
#endif
           << point[0] << ' ' << setw(width) << point[1] << ' ' << setw(width) << point[2] << ' '
           << setw(width) << diameter / 2 << setw(width);
    myfile << std::to_string(parentId) << '\n';
}
void writeHeader(std::ofstream& myfile) {
    using std::setw;

    myfile << "# " << morphio::mut::writer::details::version_string() << '\n';
    myfile << "# index" << setw(9) << "type" << setw(10) << 'X' << setw(13) << 'Y' << setw(13)
           << 'Z' << setw(13) << "radius" << setw(13) << "parent" << '\n';
}

void raiseIfUnifurcations(const morphio::mut::Morphology& morph) {
    for (auto it = morph.depth_begin(); it != morph.depth_end(); ++it) {
        std::shared_ptr<morphio::mut::Section> section_ = *it;
        if (section_->isRoot()) {
            continue;
        }

        unsigned int parentId = section_->parent()->id();

        auto parent = section_->parent();
        bool isUnifurcation = parent->children().size() == 1;

        if (isUnifurcation) {
            throw morphio::WriterError(
                morphio::readers::ErrorMessages().ERROR_ONLY_CHILD_SWC_WRITER(parentId));
        }
    }
}

/* Only skip duplicate if it has the same diameter */
bool _skipDuplicate(const std::shared_ptr<morphio::mut::Section>& section) {
    return section->diameters().front() == section->parent()->diameters().back();
}

}  // namespace

namespace morphio {
namespace mut {
namespace writer {

void swc(const Morphology& morphology, const std::string& filename) {
    const auto& soma = morphology.soma();
    const auto& soma_points = soma->points();
    if (soma_points.empty()) {
        if (morphology.rootSections().empty()) {
            printError(Warning::WRITE_EMPTY_MORPHOLOGY,
                       readers::ErrorMessages().WARNING_WRITE_EMPTY_MORPHOLOGY());
            return;
        }
        printError(Warning::WRITE_NO_SOMA, readers::ErrorMessages().WARNING_WRITE_NO_SOMA());
    } else if (soma->type() == SOMA_UNDEFINED) {
        printError(Warning::WRITE_UNDEFINED_SOMA,
                   readers::ErrorMessages().WARNING_UNDEFINED_SOMA());
    } else if (!(soma->type() == SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS ||
                 soma->type() == SomaType::SOMA_CYLINDERS ||
                 soma->type() == SomaType::SOMA_SINGLE_POINT)) {
        printError(Warning::SOMA_NON_CYLINDER_OR_POINT,
                   readers::ErrorMessages().WARNING_SOMA_NON_CYLINDER_OR_POINT());
    } else if (soma->type() == SomaType::SOMA_SINGLE_POINT && soma_points.size() != 1) {
        throw WriterError(readers::ErrorMessages().ERROR_SOMA_INVALID_SINGLE_POINT());
    } else if (soma->type() == SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS &&
               soma_points.size() != 3) {
        throw WriterError(readers::ErrorMessages().ERROR_SOMA_INVALID_THREE_POINT_CYLINDER());
    }

    details::checkSomaHasSameNumberPointsDiameters(*soma);

    if (details::hasPerimeterData(morphology)) {
        throw WriterError(readers::ErrorMessages().ERROR_PERIMETER_DATA_NOT_WRITABLE());
    }

    raiseIfUnifurcations(morphology);

    if (!morphology.mitochondria().rootSections().empty()) {
        printError(Warning::MITOCHONDRIA_WRITE_NOT_SUPPORTED,
                   readers::ErrorMessages().WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED());
    }

    std::ofstream myfile(filename);
    writeHeader(myfile);

    int segmentIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;

    const auto& soma_diameters = soma->diameters();

    if (soma->type() == SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS) {
        const std::array<Point, 3> points = {
            soma_points[0],
            soma_points[1],
            soma_points[2],
        };

        const morphio::details::ThreePointSomaStatus status =
            morphio::details::checkNeuroMorphoSoma(points, soma_diameters[0] / 2);

        if (status != morphio::details::ThreePointSomaStatus::Conforms) {
            std::stringstream stream;
            stream << status;
            printError(Warning::SOMA_NON_CONFORM,
                       readers::ErrorMessages().WARNING_NEUROMORPHO_SOMA_NON_CONFORM(stream.str()));
        }

        writeLine(myfile, 1, -1, SECTION_SOMA, soma_points[0], soma_diameters[0]);
        writeLine(myfile, 2, 1, SECTION_SOMA, soma_points[1], soma_diameters[1]);
        writeLine(myfile, 3, 1, SECTION_SOMA, soma_points[2], soma_diameters[2]);
        segmentIdOnDisk += 3;
    } else {
        for (unsigned int i = 0; i < soma_points.size(); ++i) {
            writeLine(myfile,
                      segmentIdOnDisk,
                      i == 0 ? -1 : segmentIdOnDisk - 1,
                      SECTION_SOMA,
                      soma_points[i],
                      soma_diameters[i]);
            ++segmentIdOnDisk;
        }
    }

    for (auto it = morphology.depth_begin(); it != morphology.depth_end(); ++it) {
        const std::shared_ptr<Section>& section = *it;
        const auto& points = section->points();
        const auto& diameters = section->diameters();

        assert(!points.empty() && "Empty section");
        bool isRootSection = section->isRoot();

        // skips duplicate point for non-root sections
        unsigned int firstPoint = ((isRootSection || !_skipDuplicate(section)) ? 0 : 1);
        for (unsigned int i = firstPoint; i < points.size(); ++i) {
            int parentIdOnDisk;
            if (i > firstPoint) {
                parentIdOnDisk = segmentIdOnDisk - 1;
            } else {
                parentIdOnDisk = (isRootSection ? (soma->points().empty() ? -1 : 1)
                                                : newIds[section->parent()->id()]);
            }

            writeLine(
                myfile, segmentIdOnDisk, parentIdOnDisk, section->type(), points[i], diameters[i]);

            ++segmentIdOnDisk;
        }
        newIds[section->id()] = segmentIdOnDisk - 1;
    }
}
}  // end namespace writer
}  // end namespace mut
}  // end namespace morphio
