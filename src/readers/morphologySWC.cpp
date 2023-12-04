/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "morphologySWC.h"

#include <cstdint>        // uint32_t
#include <memory>         // std::shared_ptr
#include <sstream>        // std::stringstream
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

#include <morphio/errorMessages.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>

#include "../shared_utils.hpp"

namespace {
// It's not clear if -1 is the only way of identifying the root section.
const int SWC_UNDEFINED_PARENT = -1;
const unsigned int SWC_ROOT = 0xFFFFFFFD;

bool _ignoreLine(const std::string& line) {
    std::size_t pos = line.find_first_not_of("\n\r\t ");
    return pos == std::string::npos || line[pos] == '#';
}

morphio::readers::Sample readSample(const char* line, unsigned int lineNumber_) {
#ifdef MORPHIO_USE_DOUBLE
    const char* const format = "%d%d%lg%lg%lg%lg%d";
#else
    const char* const format = "%d%d%f%f%f%f%d";
#endif

    morphio::floatType radius = -1.;
    int int_type = -1;
    morphio::readers::Sample sample;
    int parentId = -1;
    int id = -1;
    sample.valid = sscanf(line,
                          format,
                          &id,
                          &int_type,
                          &sample.point[0],
                          &sample.point[1],
                          &sample.point[2],
                          &radius,
                          &parentId) == 7;

    if (id < 0) {
        throw morphio::RawDataError("Negative IDs are not supported");
    }
    sample.id = static_cast<unsigned int>(id);

    if (parentId < -1) {
        throw morphio::RawDataError("Negative ParentIDs are not supported");
    } else if (parentId == SWC_UNDEFINED_PARENT) {
        sample.parentId = SWC_ROOT;
    } else {
        sample.parentId = static_cast<unsigned int>(parentId);
    }

    sample.type = static_cast<morphio::SectionType>(int_type);
    sample.diameter = radius * 2;  // The point array stores diameters.
    sample.lineNumber = lineNumber_;
    return sample;
}

}  // unnamed namespace

namespace morphio {
namespace readers {
namespace swc {
/**
   Parsing SWC according to this specification:
   http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
**/
class SWCBuilder
{
  public:
    explicit SWCBuilder(const std::string& path)
        : err(path) {}

    void _readSamples(const std::string& contents) {
        std::stringstream stream{contents};
        unsigned int lineNumber = 0;
        std::string line;
        while (!std::getline(stream, line).fail()) {
            ++lineNumber;

            if (line.empty() || _ignoreLine(line)) {
                continue;
            }

            const auto& sample = readSample(line.data(), lineNumber);

            if (!sample.valid) {
                throw RawDataError(err.ERROR_LINE_NON_PARSABLE(lineNumber));
            }

            if (sample.type >= SECTION_OUT_OF_RANGE_START || sample.type <= 0) {
                throw RawDataError(err.ERROR_UNSUPPORTED_SECTION_TYPE(lineNumber, sample.type));
            }

            if (samples.count(sample.id) > 0) {
                throw RawDataError(err.ERROR_REPEATED_ID(samples[sample.id], sample));
            }

            samples[sample.id] = sample;
            children[sample.parentId].push_back(sample.id);

            if (sample.type == SECTION_SOMA) {
                lastSomaPoint = sample.id;
            }
        }
    }

    /**
       Are considered potential somata all sample
       with parentId == SWC_ROOT and sample.type == SECTION_SOMA
     **/
    std::vector<Sample> _potentialSomata() {
        std::vector<Sample> somata;
        for (auto id : children[SWC_ROOT]) {
            if (samples[id].type == SECTION_SOMA) {
                somata.push_back(samples[id]);
            }
        }
        return somata;
    }

    void raiseIfBrokenSoma(const Sample& sample) {
        if (sample.type != SECTION_SOMA) {
            return;
        }

        if (sample.parentId != SWC_ROOT && !children[sample.id].empty()) {
            std::vector<Sample> soma_bifurcations;
            for (unsigned int id : children[sample.id]) {
                if (samples[id].type == SECTION_SOMA) {
                    soma_bifurcations.push_back(samples[id]);
                } else {
                    neurite_wrong_root.push_back(samples[id]);
                }
            }

            if (soma_bifurcations.size() > 1) {
                throw morphio::SomaError(err.ERROR_SOMA_BIFURCATION(sample, soma_bifurcations));
            }
        }

        if (sample.parentId != SWC_ROOT && samples.count(sample.parentId) > 0 &&
            samples[sample.parentId].type != SECTION_SOMA) {
            throw morphio::SomaError(err.ERROR_SOMA_WITH_NEURITE_PARENT(sample));
        }
    }

    void raiseIfSelfParent(const Sample& sample) {
        if (sample.parentId == sample.id) {
            throw morphio::RawDataError(err.ERROR_SELF_PARENT(sample));
        }
    }

    void warnIfDisconnectedNeurite(const Sample& sample) {
        if (sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA) {
            printError(Warning::DISCONNECTED_NEURITE, err.WARNING_DISCONNECTED_NEURITE(sample));
        }
    }

    void checkSoma() {
        auto somata = _potentialSomata();

        if (somata.size() > 1) {
            throw morphio::SomaError(err.ERROR_MULTIPLE_SOMATA(somata));
        }

        if (somata.empty()) {
            printError(Warning::NO_SOMA_FOUND, err.WARNING_NO_SOMA_FOUND());
        } else {
            for (const auto& sample_pair : samples) {
                const auto& sample = sample_pair.second;
                warnIfDisconnectedNeurite(sample);
            }
        }
    }

    void raiseIfNoParent(const Sample& sample) {
        if (sample.parentId != SWC_ROOT && samples.count(sample.parentId) == 0) {
            throw morphio::MissingParentError(err.ERROR_MISSING_PARENT(sample));
        }
    }

    void warnIfZeroDiameter(const Sample& sample) {
        if (sample.diameter < morphio::epsilon) {
            printError(Warning::ZERO_DIAMETER, err.WARNING_ZERO_DIAMETER(sample));
        }
    }

    bool isRootPoint(const Sample& sample) {
        bool isOrphanNeurite = sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA;
        return isOrphanNeurite ||
               (sample.type != SECTION_SOMA &&
                samples[sample.parentId].type == SECTION_SOMA);  // Exclude soma bifurcations
    }

    bool isSectionStart(const Sample& sample) {
        return (isRootPoint(sample) ||
                (sample.parentId != SWC_ROOT &&
                 isSectionEnd(samples[sample.parentId])));  // Standard section
    }

    bool isSectionEnd(const Sample& sample) {
        return sample.id == lastSomaPoint ||        // End of soma
               children[sample.id].empty() ||       // Reached leaf
               (children[sample.id].size() >= 2 &&  // Reached neurite bifurcation
                sample.type != SECTION_SOMA);
    }

    template <typename T>
    void appendSample(const std::shared_ptr<T>& somaOrSection, const Sample& sample) {
        somaOrSection->points().push_back(sample.point);
        somaOrSection->diameters().push_back(sample.diameter);
    }

    std::vector<unsigned int> constructDepthFirstSamples() {
        std::vector<unsigned int> ret;
        ret.reserve(samples.size());
        const auto pushChildren = [&](const auto& f, unsigned int id) -> void {
            for (unsigned int childId : children[id]) {
                ret.push_back(childId);
                f(f, childId);
            }
        };

        pushChildren(pushChildren, SWC_ROOT);

        return ret;
    }

    void raiseIfNonConform(const Sample& sample) {
        raiseIfSelfParent(sample);
        raiseIfBrokenSoma(sample);
        raiseIfNoParent(sample);
        warnIfZeroDiameter(sample);
    }

    SomaType somaType() {
        switch (morph.soma()->points().size()) {
        case 0: {
            return SOMA_UNDEFINED;
        }
        case 1: {
            return SOMA_SINGLE_POINT;
        }
        case 2: {
            return SOMA_CYLINDERS;
        }
        // NeuroMorpho format is characterized by a 3 points soma
        // with a bifurcation at soma root
        case 3: {
            uint32_t somaRootId = children[SWC_ROOT][0];
            const auto& somaChildren = children[somaRootId];

            std::vector<Sample> children_soma_points;
            for (unsigned int child : somaChildren) {
                if (this->samples[child].type == SECTION_SOMA) {
                    children_soma_points.push_back(this->samples[child]);
                }
            }

            if (children_soma_points.size() == 2) {
                if (!ErrorMessages::isIgnored(Warning::SOMA_NON_CONFORM)) {
                    const std::array<Point, 3> points = {
                        samples[somaRootId].point,
                        children_soma_points[0].point,
                        children_soma_points[1].point,
                    };
                    details::ThreePointSomaStatus status =
                        details::checkNeuroMorphoSoma(points, samples[somaRootId].diameter / 2);

                    if (status != details::ThreePointSomaStatus::Conforms) {
                        std::stringstream stream;
                        stream << status;
                        printError(Warning::SOMA_NON_CONFORM,
                                   err.WARNING_NEUROMORPHO_SOMA_NON_CONFORM(stream.str()));
                    }
                }

                return SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
            }
            return SOMA_CYLINDERS;
        }
        default:
            return SOMA_CYLINDERS;
        }
    }

    Property::Properties buildProperties(const std::string& contents, unsigned int options) {
        _readSamples(contents);

        for (const auto& sample_pair : samples) {
            const auto& sample = sample_pair.second;
            raiseIfNonConform(sample);
        }

        checkSoma();

        // The process might occasionally creates empty section before
        // filling them so the warning is ignored
        bool originalIsIgnored = err.isIgnored(morphio::Warning::APPENDING_EMPTY_SECTION);
        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, true);

        std::vector<unsigned int> depthFirstSamples = constructDepthFirstSamples();
        for (const auto id : depthFirstSamples) {
            const Sample& sample = samples[id];

            // Bifurcation right at the start
            if (isRootPoint(sample) && isSectionEnd(sample)) {
                continue;
            }

            if (isSectionStart(sample)) {
                _processSectionStart(sample);
            } else if (sample.type != SECTION_SOMA) {
                swcIdToSectionId[sample.id] =
                    swcIdToSectionId[static_cast<unsigned int>(sample.parentId)];
            }

            if (sample.type == SECTION_SOMA) {
                appendSample(morph.soma(), sample);
            } else {
                appendSample(morph.section(swcIdToSectionId.at(sample.id)), sample);
            }
        }

        if (morph.soma()->points().size() == 3 && !neurite_wrong_root.empty()) {
            printError(morphio::WRONG_ROOT_POINT, err.WARNING_WRONG_ROOT_POINT(neurite_wrong_root));
        }

        morph.applyModifiers(options);

        Property::Properties properties = morph.buildReadOnly();
        properties._cellLevel._somaType = somaType();

        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, originalIsIgnored);

        return properties;
    }

    /*
    Append last point of previous section if current section is not a root section
    Update the parent ID of the new section
    */
    void _processSectionStart(const Sample& sample) {
        Property::PointLevel properties;

        uint32_t id = 0;

        if (isRootPoint(sample)) {
            id = morph.appendRootSection(properties, sample.type)->id();
        } else {
            // Duplicating last point of previous section if there is not already a duplicate
            auto parentId = static_cast<unsigned int>(sample.parentId);
            if (sample.point != samples[parentId].point) {
                properties._points.push_back(samples[parentId].point);
                properties._diameters.push_back(samples[parentId].diameter);
            }

            // Handle the case, bifurcatation at root point
            if (isRootPoint(samples[parentId])) {
                id = morph.appendRootSection(properties, sample.type)->id();
            } else {
                id = morph.section(swcIdToSectionId[parentId])
                         ->appendSection(properties, sample.type)
                         ->id();
            }
        }

        swcIdToSectionId[sample.id] = id;
    }

  private:
    // Dictionary: SWC Id of the last point of a section to morphio::mut::Section ID
    std::unordered_map<uint32_t, uint32_t> swcIdToSectionId;

    // Neurite that do not have parent ID = 1, allowed for soma contour, not
    // 3-pts soma
    std::vector<Sample> neurite_wrong_root;

    unsigned int lastSomaPoint = 0;
    std::unordered_map<unsigned int, std::vector<unsigned int>> children;
    std::unordered_map<unsigned int, Sample> samples;
    mut::Morphology morph;
    ErrorMessages err;
};

Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options) {
    auto properties = SWCBuilder(path).buildProperties(contents, options);

    properties._cellLevel._cellFamily = NEURON;
    properties._cellLevel._version = {"swc", 1, 0};
    return properties;
}

}  // namespace swc
}  // namespace readers
}  // namespace morphio
