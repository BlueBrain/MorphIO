#include "morphologySWC.h"

#include <algorithm>      // std::transform
#include <cstdint>        // uint32_t
#include <iterator>       // std::back_inserter
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

namespace {

struct SWCSample {
    SWCSample() = default;  // XXX
    morphio::floatType diameter = -1.;
    bool valid = false;
    morphio::Point point{};
    morphio::SectionType type = morphio::SECTION_UNDEFINED;
    int parentId = -1;
    unsigned int id = 0;
    unsigned int lineNumber = 0;

    morphio::readers::Sample toSample() const {
        morphio::readers::Sample sample;
        sample.parentId = parentId;
        sample.id = id;
        sample.lineNumber = lineNumber;
        return sample;
    }
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
    const char* const format = "%u%d%lg%lg%lg%lg%d";
#else
    const char* const format = "%u%d%f%f%f%f%d";
#endif

    morphio::floatType radius = -1.;
    int int_type = -1;
    SWCSample sample;
    sample.valid = sscanf(line,
                          format,
                          &sample.id,
                          &int_type,
                          &sample.point[0],
                          &sample.point[1],
                          &sample.point[2],
                          &radius,
                          &sample.parentId) == 7;

    sample.type = static_cast<morphio::SectionType>(int_type);
    sample.diameter = radius * 2;  // The point array stores diameters.
    sample.lineNumber = lineNumber_;
    return sample;
}

}  // unnamed namespace

namespace morphio {
namespace readers {
namespace swc {
// It's not clear if -1 is the only way of identifying a root section.
const int SWC_UNDEFINED_PARENT = -1;

/**
   Parsing SWC according to this specification:
   http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
**/
class SWCBuilder
{
  public:
    explicit SWCBuilder(const std::string& path)
        : err(path) {
    }

    void _readSamples(const std::string& contents) {
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
                throw RawDataError(err.ERROR_LINE_NON_PARSABLE(lineNumber));
            }

            if (sample.type >= SECTION_OUT_OF_RANGE_START || sample.type <= 0) {
                throw RawDataError(err.ERROR_UNSUPPORTED_SECTION_TYPE(lineNumber, sample.type));
            }

            if (samples.count(sample.id) > 0) {
                throw RawDataError(
                    err.ERROR_REPEATED_ID(samples[sample.id].toSample(), sample.toSample()));
            }

            samples[sample.id] = sample;
            children[sample.parentId].push_back(sample.id);

            if (sample.type == SECTION_SOMA) {
                lastSomaPoint = static_cast<int>(sample.id);
            }
        }
    }

    /**
       Are considered potential somata all sample
       with parentId == -1 and sample.type == SECTION_SOMA
     **/
    std::vector<SWCSample> _potentialSomata() {
        std::vector<SWCSample> somata;
        for (auto id : children[-1]) {
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

        if (sample.parentId != -1 && !children[static_cast<int>(sample.id)].empty()) {
            std::vector<SWCSample> soma_bifurcations;
            for (auto id : children[static_cast<int>(sample.id)]) {
                if (samples[id].type == SECTION_SOMA) {
                    soma_bifurcations.push_back(samples[id]);
                } else {
                    neurite_wrong_root.push_back(samples[id]);
                }
            }

            if (soma_bifurcations.size() > 1) {
                throw morphio::SomaError(
                    err.ERROR_SOMA_BIFURCATION(sample.lineNumber,
                                               gatherLineNumbers(soma_bifurcations)));
            }
        }

        if (sample.parentId != -1 &&
            samples[static_cast<unsigned int>(sample.parentId)].type != SECTION_SOMA) {
            throw morphio::SomaError(err.ERROR_SOMA_WITH_NEURITE_PARENT(sample.lineNumber));
        }
    }

    void raiseIfSelfParent(const SWCSample& sample) {
        if (sample.parentId == static_cast<int>(sample.id)) {
            throw morphio::RawDataError(err.ERROR_SELF_PARENT(sample.lineNumber));
        }
    }

    void warnIfDisconnectedNeurite(const SWCSample& sample) {
        if (sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA) {
            printError(Warning::DISCONNECTED_NEURITE,
                       err.WARNING_DISCONNECTED_NEURITE(sample.lineNumber));
        }
    }

    void checkSoma() {
        auto somata = _potentialSomata();

        if (somata.size() > 1) {
            const auto lineNumbers = gatherLineNumbers(somata);
            throw morphio::SomaError(err.ERROR_MULTIPLE_SOMATA(lineNumbers));
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

    void raiseIfNoParent(const SWCSample& sample) {
        if (sample.parentId > -1 &&
            samples.count(static_cast<unsigned int>(sample.parentId)) == 0) {
            throw morphio::MissingParentError(err.ERROR_MISSING_PARENT(sample.toSample()));
        }
    }

    void warnIfZeroDiameter(const SWCSample& sample) {
        if (sample.diameter < morphio::epsilon) {
            printError(Warning::ZERO_DIAMETER, err.WARNING_ZERO_DIAMETER(sample.lineNumber));
        }
    }

    /**
       A neurite which is not attached to the soma
    **/
    bool isOrphanNeurite(const SWCSample& sample) {
        return (sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA);
    }

    bool isRootPoint(const SWCSample& sample) {
        return isOrphanNeurite(sample) ||
               (sample.type != SECTION_SOMA &&
                samples[static_cast<unsigned int>(sample.parentId)].type ==
                    SECTION_SOMA);  // Exclude soma bifurcations
    }

    bool isSectionStart(const SWCSample& sample) {
        return (isRootPoint(sample) ||
                (sample.parentId > -1 &&
                 isSectionEnd(samples[static_cast<unsigned int>(sample.parentId)])));  // Standard
                                                                                       // section
    }

    bool isSectionEnd(const SWCSample& sample) {
        int id = static_cast<int>(sample.id);
        return id == lastSomaPoint ||        // End of soma
               children[id].empty() ||       // Reached leaf
               (children[id].size() >= 2 &&  // Reached neurite
                                             // bifurcation
                sample.type != SECTION_SOMA);
    }

    template <typename T>
    void appendSample(const std::shared_ptr<T>& somaOrSection, const SWCSample& sample) {
        somaOrSection->points().push_back(sample.point);
        somaOrSection->diameters().push_back(sample.diameter);
    }

    void _pushChildren(std::vector<unsigned int>& vec, int32_t id) {
        for (unsigned int childId : children[id]) {
            vec.push_back(childId);
            _pushChildren(vec, static_cast<int>(childId));
        }
    }

    void raiseIfNonConform(const SWCSample& sample) {
        raiseIfSelfParent(sample);
        raiseIfBrokenSoma(sample);
        raiseIfNoParent(sample);
        warnIfZeroDiameter(sample);
    }

    void _checkNeuroMorphoSoma(const SWCSample& root, const std::vector<SWCSample>& _children) {
        floatType x = root.point[0];
        floatType y = root.point[1];
        floatType z = root.point[2];
        floatType d = root.diameter;
        floatType r = root.diameter / 2;
        const auto& child1 = _children[0];
        const auto& child2 = _children[1];

        // whether the soma should be checked for the special case of 3 point soma
        // for details see https://github.com/BlueBrain/MorphIO/issues/273
        bool isSuited = std::fabs(child1.diameter - d) < morphio::epsilon &&
                        std::fabs(child2.diameter - d) < morphio::epsilon &&
                        std::fabs(child1.point[0] - x) < morphio::epsilon &&
                        std::fabs(child2.point[0] - x) < morphio::epsilon &&
                        std::fabs(child1.point[2] - z) < morphio::epsilon &&
                        std::fabs(child2.point[2] - z) < morphio::epsilon;
        if (!isSuited) {
            return;
        }
        // If the 2nd and the 3rd point have the same x,z,d values then the only valid soma is:
        // 1 1 x   y   z r -1
        // 2 1 x (y-r) z r  1
        // 3 1 x (y+r) z r  1
        if (child1.point[0] != x || child2.point[0] != x || child1.point[1] != y - r ||
            child2.point[1] != y + r || child1.point[2] != z || child2.point[2] != z ||
            child1.diameter != d || child2.diameter != d) {
            printError(Warning::SOMA_NON_CONFORM,
                       err.WARNING_NEUROMORPHO_SOMA_NON_CONFORM(root.point,
                                                                root.diameter,
                                                                child1.point,
                                                                child1.diameter,
                                                                child2.point,
                                                                child2.diameter));
        }
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
            uint32_t somaRootId = children[-1][0];
            auto& somaChildren = children[static_cast<int>(somaRootId)];

            std::vector<SWCSample> children_soma_points;
            for (auto child : somaChildren) {
                if (this->samples[child].type == SECTION_SOMA) {
                    children_soma_points.push_back(this->samples[child]);
                }
            }

            if (children_soma_points.size() == 2) {
                //  NeuroMorpho is the main provider of morphologies, but they
                //  with SWC as their default file format: they convert all
                //  uploads to SWC.  In the process of conversion, they turn all
                //  somas into their custom 'Three-point soma representation':
                //   http://neuromorpho.org/SomaFormat.html

                if (!ErrorMessages::isIgnored(Warning::SOMA_NON_CONFORM)) {
                    _checkNeuroMorphoSoma(this->samples[somaRootId], children_soma_points);
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

        std::vector<unsigned int> depthFirstSamples;
        _pushChildren(depthFirstSamples, -1);
        for (const auto id : depthFirstSamples) {
            const auto& sample = samples[id];

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
            printError(morphio::WRONG_ROOT_POINT,
                       err.WARNING_WRONG_ROOT_POINT(gatherLineNumbers(neurite_wrong_root)));
        }

        morph.applyModifiers(options);

        Property::Properties properties = morph.buildReadOnly();
        properties._cellLevel._somaType = somaType();

        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, originalIsIgnored);

        return properties;
    }

    /**
       - Append last point of previous section if current section is not a root section
       - Update the parent ID of the new section
    **/
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

    int lastSomaPoint = -1;
    std::unordered_map<int32_t, std::vector<uint32_t>> children;
    std::unordered_map<uint32_t, SWCSample> samples;
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
