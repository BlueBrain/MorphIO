#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <list>
#include <sstream>

#include "morphologySWC.h"
#include <morphio/errorMessages.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>

namespace {
bool _ignoreLine(const std::string& line)
{
    std::size_t pos = line.find_first_not_of("\n\r\t ");
    return pos == std::string::npos || line[pos] == '#';
}

} // unnamed namespace

namespace morphio {
namespace plugin {
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
    SWCBuilder(const std::string& uri)
        : uri(uri)
        , err(uri)
        , debugInfo(uri)
    {
        _readSamples();

        for (auto sample_pair : samples) {
            const auto& sample = sample_pair.second;
            raiseIfNonConform(sample);
        }

        checkSoma();
    }

    void _readSamples()
    {
        std::ifstream file(uri.c_str());
        if (file.fail())
            LBTHROW(morphio::RawDataError(err.ERROR_OPENING_FILE()));

        size_t lineNumber = 0;
        std::string line;
        while (!std::getline(file, line).fail()) {
            ++lineNumber;

            if (line.empty() || _ignoreLine(line))
                continue;

            const auto& sample = Sample(line.data(), lineNumber);
            if (!sample.valid)
                LBTHROW(morphio::RawDataError(
                    err.ERROR_LINE_NON_PARSABLE(lineNumber)));

            if (samples.count(sample.id) > 0)
                LBTHROW(morphio::RawDataError(
                    err.ERROR_REPEATED_ID(samples[sample.id], sample)));

            samples[sample.id] = sample;
            children[sample.parentId].push_back(sample.id);

            if (sample.type == SECTION_SOMA) {
                lastSomaPoint = sample.id;
            }
        }
        file.close();
    }

    /**
       Are considered potential somata all sample
       with parentId == -1 and sample.type == SECTION_SOMA
     **/
    std::vector<Sample> _potentialSomata()
    {
        std::vector<Sample> somata;
        for (auto id : children[-1]) {
            if (samples[id].type == SECTION_SOMA)
                somata.push_back(samples[id]);
        }
        return somata;
    }

    void raiseIfBrokenSoma(const Sample& sample)
    {
        if (sample.type != SECTION_SOMA)
            return;

        if (sample.parentId != -1 && children[sample.id].size() > 0) {
            std::vector<Sample> soma_bifurcations;
            for (auto id : children[sample.id]) {
                if (samples[id].type == SECTION_SOMA)
                    soma_bifurcations.push_back(samples[id]);
                else
                    neurite_wrong_root.push_back(samples[id]);
            }

            if (soma_bifurcations.size() > 1)
                LBTHROW(morphio::SomaError(
                    err.ERROR_SOMA_BIFURCATION(sample, soma_bifurcations)));
        }

        if (sample.parentId != -1 && samples[sample.parentId].type != SECTION_SOMA)
            LBTHROW(
                morphio::SomaError(err.ERROR_SOMA_WITH_NEURITE_PARENT(sample)));
    }

    void raiseIfSelfParent(const Sample& sample)
    {
        if (sample.parentId == sample.id)
            LBTHROW(morphio::RawDataError(err.ERROR_SELF_PARENT(sample)));
    }

    void warnIfDisconnectedNeurite(const Sample& sample)
    {
        if (sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA)
            LBERROR(Warning::DISCONNECTED_NEURITE,
                err.WARNING_DISCONNECTED_NEURITE(sample));
    }

    void checkSoma()
    {
        auto somata = _potentialSomata();

        if (somata.size() > 1)
            LBTHROW(morphio::SomaError(err.ERROR_MULTIPLE_SOMATA(somata)));

        if (somata.size() == 0)
            LBERROR(Warning::NO_SOMA_FOUND, err.WARNING_NO_SOMA_FOUND());
    }

    void raiseIfNoParent(const Sample& sample)
    {
        if (sample.parentId > -1 && samples.count(sample.parentId) == 0)
            LBTHROW(
                morphio::MissingParentError(err.ERROR_MISSING_PARENT(sample)));
    }

    /**
       A neurite which is not attached to the soma
    **/
    inline bool isOrphanNeurite(const Sample& sample)
    {
        return (sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA);
    }

    inline bool isRootSection(const Sample& sample)
    {
        return isOrphanNeurite(sample) || (samples[sample.parentId].type == SECTION_SOMA && sample.type != SECTION_SOMA); // Exclude soma bifurcations
    }

    inline bool isSectionStart(const Sample& sample)
    {
        return (isRootSection(sample) || (sample.parentId > -1 && isSectionEnd(samples[sample.parentId]))); // Standard section
    }

    inline bool isSectionEnd(const Sample& sample)
    {
        return sample.id == lastSomaPoint ||       // End of soma
               children[sample.id].size() == 0 ||  // Reached leaf
               (children[sample.id].size() >= 2 && // Reached neurite
                   // bifurcation
                   sample.type != SECTION_SOMA);
    }

    template <typename T>
    void appendSample(std::shared_ptr<T> somaOrSection, const Sample& sample)
    {
        debugInfo.setLineNumber(sample.id, sample.lineNumber);
        somaOrSection->points().push_back(sample.point);
        somaOrSection->diameters().push_back(sample.diameter);
    }

    void _pushChildren(std::vector<int32_t>& vec, int32_t id)
    {
        for (auto childId : children[id]) {
            vec.push_back(childId);
            _pushChildren(vec, childId);
        }
    }

    void raiseIfNonConform(const Sample& sample)
    {
        raiseIfSelfParent(sample);
        raiseIfBrokenSoma(sample);
        raiseIfNoParent(sample);
        warnIfDisconnectedNeurite(sample);
    }

    void _checkNeuroMorphoSoma(const Sample& root,
        const std::vector<Sample>& children)
    {
        // The only valid neuro-morpho soma is:
        // 1 1 x   y   z r -1
        // 2 1 x (y-r) z r  1
        // 3 1 x (y+r) z r  1

        float x = root.point[0], y = root.point[1], z = root.point[2],
              d = root.diameter, r = root.diameter / 2.;
        const Sample& child1 = children[0];
        const Sample& child2 = children[1];

        if (child1.point[0] != x || child2.point[0] != x || child1.point[1] != y - r || child2.point[1] != y + r || child1.point[2] != z || child2.point[2] != z || child1.diameter != d || child2.diameter != d) {
            LBERROR(Warning::SOMA_NON_CONFORM,
                err.WARNING_NEUROMORPHO_SOMA_NON_CONFORM(root, child1,
                    child2));
        }
    }

    SomaType somaType()
    {
        switch (morph.soma()->points().size()) {
        case 0: {
            return SOMA_UNDEFINED;
        }
        case 1: {
            return SOMA_SINGLE_POINT;
        }
        case 2: {
            return SOMA_UNDEFINED;
        }
        // NeuroMorpho format is characterized by a 3 points soma
        // with a bifurcation at soma root
        case 3: {
            uint32_t somaRootId = children[-1][0];
            auto& somaChildren = children[somaRootId];

            std::vector<Sample> children_soma_points;
            for (auto child : somaChildren) {
                if (this->samples[child].type == SECTION_SOMA)
                    children_soma_points.push_back(this->samples[child]);
            }

            if (somaChildren.size() == 2) {
                //  NeuroMorpho is the main provider of morphologies, but they
                //  with SWC as their default file format: they convert all
                //  uploads to SWC.  In the process of conversion, they turn all
                //  somas into their custom 'Three-point soma representation':
                //   http://neuromorpho.org/SomaFormat.html

                if (!ErrorMessages::isIgnored(Warning::SOMA_NON_CONFORM))
                    _checkNeuroMorphoSoma(this->samples[somaRootId],
                        children_soma_points);

                return SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
            }
            return SOMA_CYLINDERS;
        }
        default:
            return SOMA_CYLINDERS;
        }
    }

    Property::Properties _buildProperties(unsigned int options)
    {
        // The process might occasionally creates empty section before
        // filling them so the warning is ignored
        bool originalIsIgnored = err.isIgnored(morphio::Warning::APPENDING_EMPTY_SECTION);
        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, true);

        std::vector<int32_t> depthFirstSamples;
        _pushChildren(depthFirstSamples, -1);
        for (const auto id : depthFirstSamples) {
            const Sample& sample = samples[id];
            if (isSectionStart(sample)) {
                _processSectionStart(sample);
            } else if (sample.type != SECTION_SOMA) {
                swcIdToSectionId[sample.id] = swcIdToSectionId[sample.parentId];
            }

            if (sample.type == SECTION_SOMA) {
                appendSample(morph.soma(), sample);
            } else {
                appendSample(morph.section(swcIdToSectionId.at(sample.id)),
                    sample);
            }
        }

        if (morph.soma()->points().size() == 3 && neurite_wrong_root.size() > 0)
            LBERROR(morphio::WRONG_ROOT_POINT,
                err.WARNING_WRONG_ROOT_POINT(neurite_wrong_root));

        morph.sanitize();
        morph.applyModifiers(options);

        Property::Properties properties = morph.buildReadOnly();
        properties._cellLevel._somaType = somaType();

        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, originalIsIgnored);

        return properties;
    }

    /**
       - Append last point of previous section if current section is not a root
    section
       - Update the parent ID of the new section
    **/
    void _processSectionStart(const Sample& sample)
    {
        Property::PointLevel properties;

        std::shared_ptr<morphio::mut::Section> section;

        if (isRootSection(sample)) {
            section = morph.appendRootSection(properties, sample.type);
        } else {
            // Duplicating last point of previous section if there is not already a duplicate
            if (sample.point != samples[sample.parentId].point) {
                properties._points.push_back(samples[sample.parentId].point);
                properties._diameters.push_back(samples[sample.parentId].diameter);
            }
            section = morph.section(swcIdToSectionId[sample.parentId])
                          ->appendSection(properties, sample.type);
        }

        swcIdToSectionId[sample.id] = section->id();
    }

private:
    // Dictionnary: SWC Id of the last point of a section to
    // morphio::mut::Section ID
    std::map<uint32_t, uint32_t> swcIdToSectionId;

    // Neurite that do not have parent ID = 1, allowed for soma contour, not
    // 3-pts soma
    std::vector<Sample> neurite_wrong_root;

    int lastSomaPoint = -1;
    std::map<int32_t, std::vector<uint32_t>> children;
    std::map<uint32_t, Sample> samples;
    mut::Morphology morph;
    std::string uri;
    ErrorMessages err;
    DebugInfo debugInfo;
};

Property::Properties load(const URI& uri, unsigned int options)
{
    auto properties = SWCBuilder(uri)._buildProperties(options);
    properties._cellLevel._cellFamily = FAMILY_NEURON;
    properties._cellLevel._version = MORPHOLOGY_VERSION_SWC_1;
    return properties;
}

} // namespace swc
} // namespace plugin
} // namespace morphio
