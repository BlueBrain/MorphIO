/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <cassert>
#include <fstream>
#include <iomanip>  // std::fixed, std::setw, std::setprecision
#include <memory>
#include <unordered_map>

#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>
#include <morphio/vector_types.h>
#include <morphio/version.h>
#include <morphio/warning_handling.h>

#include <highfive/H5DataSet.hpp>
#include <highfive/H5File.hpp>
#include <highfive/H5Object.hpp>

#include "../error_message_generation.h"
#include "../shared_utils.hpp"
#include "writer_utils.h"


namespace {
bool anySamplesMatch(const morphio::Point& point,
                     const morphio::floatType diameter,
                     const morphio::Points& points,
                     const std::vector<morphio::floatType>& diameters) {
    for (size_t i = 0; i < points.size(); ++i) {
        if (std::fabs(diameters[i] - diameter) < morphio::epsilon &&
            std::fabs(points[0][0] - point[0]) < morphio::epsilon &&
            std::fabs(points[0][1] - point[1]) < morphio::epsilon &&
            std::fabs(points[0][2] - point[2]) < morphio::epsilon) {
            return true;
        }
    }
    return false;
}

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

int writeSoma(std::ofstream& myfile,
              const std::shared_ptr<morphio::mut::Soma>& soma,
              std::shared_ptr<morphio::WarningHandler> handler) {
    using morphio::enums::SectionType;

    const auto& soma_points = soma->points();
    const auto& soma_diameters = soma->diameters();

    int startIdOnDisk = 1;
    if (soma->type() == morphio::SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS) {
        const std::array<morphio::Point, 3> points = {
            soma_points[0],
            soma_points[1],
            soma_points[2],
        };

        const morphio::details::ThreePointSomaStatus status =
            morphio::details::checkNeuroMorphoSoma(points, soma_diameters[0] / 2);

        if (status != morphio::details::ThreePointSomaStatus::Conforms) {
            std::stringstream stream;
            stream << status;
            handler->emit(std::make_shared<morphio::SomaNonConform>("", stream.str()));
        }

        writeLine(myfile, 1, -1, SectionType::SECTION_SOMA, soma_points[0], soma_diameters[0]);
        writeLine(myfile, 2, 1, SectionType::SECTION_SOMA, soma_points[1], soma_diameters[1]);
        writeLine(myfile, 3, 1, SectionType::SECTION_SOMA, soma_points[2], soma_diameters[2]);
        startIdOnDisk += 3;
    } else {
        for (unsigned int i = 0; i < soma_points.size(); ++i) {
            writeLine(myfile,
                      startIdOnDisk,
                      i == 0 ? -1 : startIdOnDisk - 1,
                      SectionType::SECTION_SOMA,
                      soma_points[i],
                      soma_diameters[i]);
            ++startIdOnDisk;
        }
    }
    return startIdOnDisk;
}

bool _sameDiameter(const std::shared_ptr<morphio::mut::Section>& section) {
    return std::fabs(section->diameters().front() - section->parent()->diameters().back()) <
           morphio::epsilon;
}

void validateSWCSoma(const morphio::mut::Morphology& morph,
                     std::shared_ptr<morphio::WarningHandler> handler) {
    using morphio::SomaType;
    using morphio::Warning;
    using morphio::WriterError;
    using morphio::details::ErrorMessages;

    const auto& soma = morph.soma();
    const auto& soma_points = soma->points();
    if (soma_points.empty()) {
        if (morph.rootSections().empty()) {
            handler->emit(std::make_shared<morphio::WriteEmptyMorphology>());
            return;
        }
        handler->emit(std::make_shared<morphio::WriteNoSoma>());

    } else if (soma->type() == morphio::SOMA_UNDEFINED) {
        handler->emit(std::make_shared<morphio::WriteUndefinedSoma>());
    } else if (!(soma->type() == SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS ||
                 soma->type() == SomaType::SOMA_CYLINDERS ||
                 soma->type() == SomaType::SOMA_SINGLE_POINT)) {
        handler->emit(std::make_shared<morphio::SomaNonCylinderOrPoint>());
    } else if (soma->type() == SomaType::SOMA_SINGLE_POINT && soma_points.size() != 1) {
        throw WriterError(ErrorMessages().ERROR_SOMA_INVALID_SINGLE_POINT());
    } else if (soma->type() == SomaType::SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS &&
               soma_points.size() != 3) {
        throw WriterError(ErrorMessages().ERROR_SOMA_INVALID_THREE_POINT_CYLINDER());
    }
}


}  // namespace

namespace morphio {
namespace mut {
namespace writer {

void swc(const Morphology& morph,
         const std::string& filename,
         std::shared_ptr<morphio::WarningHandler> handler) {
    if (details::emptyMorphology(morph, handler)) {
        throw morphio::WriterError(
            morphio::details::ErrorMessages(filename).ERROR_EMPTY_MORPHOLOGY());
    }

    const std::shared_ptr<Soma>& soma = morph.soma();
    validateSWCSoma(morph, handler);
    details::checkSomaHasSameNumberPointsDiameters(*soma);
    details::validateHasNoMitochondria(morph, handler);
    details::validateHasNoPerimeterData(morph);

    std::ofstream myfile(filename);
    writeHeader(myfile);

    int segmentIdOnDisk = writeSoma(myfile, soma, handler);

    std::unordered_map<uint32_t, int32_t> newIds;
    for (auto it = morph.depth_begin(); it != morph.depth_end(); ++it) {
        const std::shared_ptr<Section>& section = *it;
        const auto& points = section->points();
        const auto& diameters = section->diameters();

        assert(!points.empty() && "Empty section");
        bool isRootSection = section->isRoot();
        if (!isRootSection) {
            bool isUnifurcation = section->parent()->children().size() == 1;
            if (isUnifurcation) {
                throw morphio::WriterError(
                    morphio::details::ErrorMessages().ERROR_ONLY_CHILD_SWC_WRITER(
                        section->parent()->id()));
            }
        }

        unsigned int firstPoint = 0;
        // When there is only a single point in a section on read, MorphIO adds a point to keep the
        // invariant that each section has a segment, which in turn has 2 points. When writing, that
        // point is dropped
        if (isRootSection &&
            anySamplesMatch(points[0], diameters[0], soma->points(), soma->diameters())) {
            firstPoint = 1;
        } else if (!isRootSection && _sameDiameter(section)) {
            // skips duplicate point for non-root sections, if they have the same diameter
            firstPoint = 1;
        }

        for (unsigned int i = firstPoint; i < points.size(); ++i) {
            int parentIdOnDisk = 0;
            if (i > firstPoint) {
                parentIdOnDisk = segmentIdOnDisk - 1;
            } else if (isRootSection) {
                if (soma->points().empty()) {
                    parentIdOnDisk = -1;
                } else {
                    parentIdOnDisk = 1;
                }
            } else {
                parentIdOnDisk = newIds[section->parent()->id()];
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
