/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "morphologySWC.h"

#include <algorithm>      // std::transform
#include <cstdint>        // uint32_t
#include <iterator>       // std::back_inserter
#include <memory>         // std::shared_ptr
#include <sstream>        // std::stringstream
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <utility>
#include <vector>         // std::vector

#include <morphio/errorMessages.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>

#include "../error_message_generation.h"
#include "../shared_utils.hpp"

namespace {
// It's not clear if -1 is the only way of identifying the root section.
const int SWC_UNDEFINED_PARENT = -1;
const unsigned int SWC_ROOT = 0xFFFFFFFD;

struct SWCSample {
    enum : unsigned int { UNKNOWN_ID = 0xFFFFFFFE };

    SWCSample() = default;  // XXX
    morphio::floatType diameter = -1.;
    bool valid = false;
    morphio::Point point{};
    morphio::SectionType type = morphio::SECTION_UNDEFINED;
    unsigned int parentId = UNKNOWN_ID;
    unsigned int id = UNKNOWN_ID;
    unsigned int lineNumber = 0;

};

std::vector<unsigned int> gatherLineNumbers(const std::vector<SWCSample>& samples) {
    std::vector<unsigned int> lineNumbers;
    std::transform(samples.begin(),
                   samples.cend(),
                   std::back_inserter(lineNumbers),
                   [](const SWCSample& sample) { return sample.lineNumber; });
    return lineNumbers;
}

bool _ignoreLine(const std::string& line) {
    std::size_t pos = line.find_first_not_of("\n\r\t ");
    return pos == std::string::npos || line[pos] == '#';
}

SWCSample readSample(const char* line, unsigned int lineNumber_) {
#ifdef MORPHIO_USE_DOUBLE
    const char* const format = "%d%d%lg%lg%lg%lg%d";
#else
    const char* const format = "%d%d%f%f%f%f%d";
#endif

    morphio::floatType radius = -1.;
    int int_type = -1;
    int parentId = -1;
    int id = -1;
    SWCSample sample;
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
    explicit SWCBuilder(std::string uri_)
        : uri(std::move(uri_)) {}

    void readSamples(const std::string& contents) {
        std::stringstream stream{contents};
        unsigned int lineNumber = 0;
        std::string line;
        while (!std::getline(stream, line).fail()) {
            ++lineNumber;

            if (line.empty() || _ignoreLine(line)) {
                continue;
            }

            const auto sample = readSample(line.data(), lineNumber);

            if (!sample.valid) {
                const auto err = details::ErrorMessages(uri);
                throw RawDataError(err.ERROR_LINE_NON_PARSABLE(lineNumber));
            }

            if (sample.type >= SECTION_OUT_OF_RANGE_START || sample.type <= 0) {
                const auto err = details::ErrorMessages(uri);
                throw RawDataError(err.ERROR_UNSUPPORTED_SECTION_TYPE(lineNumber, sample.type));
            }

            if (samples.count(sample.id) > 0) {
                const auto err = details::ErrorMessages(uri);
                auto original = samples[sample.id];
                throw RawDataError(
                    err.ERROR_REPEATED_ID(original.id, original.lineNumber, sample.lineNumber));
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
    std::vector<SWCSample> _potentialSomata() {
        std::vector<SWCSample> somata;
        for (auto id : children[SWC_ROOT]) {
            if (samples[id].type == SECTION_SOMA) {
                somata.push_back(samples[id]);
            }
        }
        return somata;
    }

    void raiseIfBrokenSoma(const SWCSample& sample) {
        if (sample.type != SECTION_SOMA) {
            return;
        }

        if (sample.parentId != SWC_ROOT && !children[sample.id].empty()) {
            std::vector<SWCSample> soma_bifurcations;
            for (unsigned int id : children[sample.id]) {
                if (samples[id].type == SECTION_SOMA) {
                    soma_bifurcations.push_back(samples[id]);
                } else {
                    neurite_wrong_root.push_back(samples[id]);
                }
            }

            if (soma_bifurcations.size() > 1) {
                const auto err = details::ErrorMessages(uri);
                throw morphio::SomaError(
                    err.ERROR_SOMA_BIFURCATION(sample.lineNumber,
                                               gatherLineNumbers(soma_bifurcations)));
            }
        }

        if (sample.parentId != SWC_ROOT && samples.count(sample.parentId) > 0 &&
            samples[sample.parentId].type != SECTION_SOMA) {
            const auto err = details::ErrorMessages(uri);
            throw morphio::SomaError(err.ERROR_SOMA_WITH_NEURITE_PARENT(sample.lineNumber));
        }
    }

    void raiseIfSelfParent(const SWCSample& sample) {
        if (sample.parentId == sample.id) {
            const auto err = details::ErrorMessages(uri);
            throw morphio::RawDataError(err.ERROR_SELF_PARENT(sample.id));
        }
    }

    void warnIfDisconnectedNeurite(const SWCSample& sample,
                                   std::shared_ptr<ErrorAndWarningHandler>& h) {
        if (sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA) {
            const auto err = details::ErrorMessages(uri);
            h->emit(Warning::DISCONNECTED_NEURITE,
                    err.WARNING_DISCONNECTED_NEURITE(sample.lineNumber));
        }
    }

    void checkSoma(std::shared_ptr<ErrorAndWarningHandler>& h) {
        auto somata = _potentialSomata();

        if (somata.size() > 1) {
            const auto err = details::ErrorMessages(uri);
            const auto lineNumbers = gatherLineNumbers(somata);
            throw morphio::SomaError(err.ERROR_MULTIPLE_SOMATA(lineNumbers));
        }

        if (somata.empty()) {
            const auto err = details::ErrorMessages(uri);
            h->emit(Warning::NO_SOMA_FOUND, err.WARNING_NO_SOMA_FOUND());
        } else {
            for (const auto& sample_pair : samples) {
                const auto& sample = sample_pair.second;
                warnIfDisconnectedNeurite(sample, h);
            }
        }
    }

    void raiseIfNoParent(const SWCSample& sample) {
        if (sample.parentId != SWC_ROOT && samples.count(sample.parentId) == 0) {
            const auto err = details::ErrorMessages(uri);
            throw morphio::MissingParentError(err.ERROR_MISSING_PARENT(
                sample.id, static_cast<int>(sample.parentId), sample.lineNumber));
        }
    }

    void warnIfZeroDiameter(const SWCSample& sample,
                            std::shared_ptr<ErrorAndWarningHandler> h
                            ) {
        if (sample.diameter < morphio::epsilon) {
            const auto err = details::ErrorMessages(uri);
            h->emit(Warning::ZERO_DIAMETER, err.WARNING_ZERO_DIAMETER(sample.lineNumber));
        }
    }

    bool isRootPoint(const SWCSample& sample) {
        bool isOrphanNeurite = sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA;
        return isOrphanNeurite ||
               (sample.type != SECTION_SOMA &&
                samples[sample.parentId].type == SECTION_SOMA);  // Exclude soma bifurcations
    }

    bool isSectionStart(const SWCSample& sample) {
        return (isRootPoint(sample) ||
                (sample.parentId != SWC_ROOT &&
                 isSectionEnd(samples[sample.parentId])));  // Standard section
    }

    bool isSectionEnd(const SWCSample& sample) {
        return sample.id == lastSomaPoint ||        // End of soma
               children[sample.id].empty() ||       // Reached leaf
               (children[sample.id].size() >= 2 &&  // Reached neurite bifurcation
                sample.type != SECTION_SOMA);
    }

    template <typename T>
    void appendSample(const std::shared_ptr<T>& somaOrSection, const SWCSample& sample) {
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

    SomaType somaType(std::shared_ptr<ErrorAndWarningHandler> h) {
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

            std::vector<SWCSample> children_soma_points;
            for (unsigned int child : somaChildren) {
                if (this->samples[child].type == SECTION_SOMA) {
                    children_soma_points.push_back(this->samples[child]);
                }
            }

            if (children_soma_points.size() == 2) {
                if (!h->isIgnored(Warning::SOMA_NON_CONFORM)) {
                    const std::array<Point, 3> points = {
                        samples[somaRootId].point,
                        children_soma_points[0].point,
                        children_soma_points[1].point,
                    };
                    details::ThreePointSomaStatus status =
                        details::checkNeuroMorphoSoma(points, samples[somaRootId].diameter / 2);

                    if (status != details::ThreePointSomaStatus::Conforms) {
                        const auto err = details::ErrorMessages(uri);
                        std::stringstream stream;
                        stream << status;
                        h->emit(Warning::SOMA_NON_CONFORM,
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

    Property::Properties buildProperties(const std::string& contents, unsigned int options,
                                         std::shared_ptr<ErrorAndWarningHandler> h
                                        ) {
        readSamples(contents);

        for (const auto& sample_pair : samples) {
            const auto& sample = sample_pair.second;
            raiseIfSelfParent(sample);
            raiseIfBrokenSoma(sample);
            raiseIfNoParent(sample);
            warnIfZeroDiameter(sample, h);
        }

        checkSoma(h);

        // The process might occasionally creates empty section before
        // filling them so the warning is ignored
        bool originalIsIgnored = h->isIgnored(morphio::Warning::APPENDING_EMPTY_SECTION);
        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, true);

        std::vector<unsigned int> depthFirstSamples = constructDepthFirstSamples();
        for (const auto id : depthFirstSamples) {
            const SWCSample& sample = samples[id];

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
            const auto err = details::ErrorMessages(uri);
            h->emit(morphio::WRONG_ROOT_POINT,
                    err.WARNING_WRONG_ROOT_POINT(gatherLineNumbers(neurite_wrong_root)));
        }

        morph.applyModifiers(options);

        Property::Properties properties = morph.buildReadOnly();
        properties._cellLevel._somaType = somaType(h);

        h->setIgnoredWarning(morphio::Warning::APPENDING_EMPTY_SECTION, originalIsIgnored);

        return properties;
    }

    /*
       append last point of previous section if current section is not a root section
       and update the parent ID of the new section
    */
    void _processSectionStart(const SWCSample& sample) {
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
    std::vector<SWCSample> neurite_wrong_root;

    unsigned int lastSomaPoint = 0;
    std::unordered_map<unsigned int, std::vector<unsigned int>> children;
    std::unordered_map<unsigned int, SWCSample> samples;
    mut::Morphology morph;
    std::string uri;
};

Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options,
                          std::shared_ptr<ErrorAndWarningHandler> h
                          ) {
    auto properties = SWCBuilder(path).buildProperties(contents, options, h);

    properties._cellLevel._cellFamily = NEURON;
    properties._cellLevel._version = {"swc", 1, 0};
    return properties;
}

}  // namespace swc
}  // namespace readers
}  // namespace morphio
