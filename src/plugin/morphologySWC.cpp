#include <cassert>
#include <fstream>
#include <list>
#include <sstream>
#include <algorithm>
#include <regex>

#include <iostream>

#include <morphio/properties.h>
#include "morphologySWC.h"
#include <morphio/mut/morphology.h>
#include <morphio/mut/soma.h>
#include <morphio/mut/section.h>

#include "errorMessages.h"

namespace morphio
{
namespace plugin
{
namespace swc
{

// It's not clear if -1 is the only way of identifying a root section.
const int SWC_UNDEFINED_PARENT = -1;



/**
   Parsing SWC according to this specification:
   http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
**/
class SWCBuilder
{
public:
    SWCBuilder(const std::string& uri) : uri(uri), err(uri) {
        _readSamples();

        for(auto sample_pair: samples){
            const auto &sample = sample_pair.second;
            raiseIfNonConform(sample);
        }

        checkSoma();
    }


    void _readSamples() {
        std::ifstream file(uri.c_str());
        if (file.fail())
            LBTHROW(morphio::RawDataError(err.ERROR_OPENING_FILE()));

        size_t lineNumber = 0;
        std::string line;
        while (!std::getline(file, line).fail())
        {
            ++lineNumber;
            if (std::regex_match (line, std::regex(" *(#.*)?") ) || line.empty())
                continue;

            const auto &sample = Sample(line.data(), lineNumber);
            if (!sample.valid)
                LBTHROW(morphio::RawDataError(err.ERROR_LINE_NON_PARSABLE(lineNumber)));

            if(samples.count(sample.id) > 0)
                LBTHROW(morphio::RawDataError(err.ERROR_REPEATED_ID(samples[sample.id], sample)));

            samples[sample.id] = sample;
            children[sample.parentId].push_back(sample.id);

            if(sample.type == SECTION_SOMA) {
                lastSomaPoint = sample.id;
            }
        }
        file.close();
    }



    /**
       Are considered potential somata all sample
       with parentId == -1 and sample.type == SECTION_SOMA
     **/
    std::vector<Sample> _potentialSomata() {
        std::vector<Sample> somata;
        for(auto id: children[-1]) {
            if(samples[id].type == SECTION_SOMA)
                somata.push_back(samples[id]);
        }
        return somata;
    }

    void raiseIfBrokenSoma(const Sample& sample) {
        if(sample.type != SECTION_SOMA)
            return;

        // Testing soma bifurcation
        if(sample.parentId != -1 && children[sample.id].size() > 1)
            LBTHROW(morphio::SomaError(err.ERROR_SOMA_BIFURCATION(sample)));

        if(sample.parentId != -1 && samples[sample.parentId].type != SECTION_SOMA)
            LBTHROW(morphio::SomaError(err.ERROR_SOMA_WITH_NEURITE_PARENT(sample)));
    }

    void raiseIfSelfParent(const Sample& sample) {
        if(sample.parentId == sample.id)
            LBTHROW(morphio::RawDataError(err.ERROR_SELF_PARENT(sample)));
    }

    void raiseIfDisconnectedNeurite(const Sample& sample) {
        if(sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA)
            LBERROR(err.WARNING_DISCONNECTED_NEURITE(sample));
    }

    void checkSoma() {
        auto somata = _potentialSomata();

        if(somata.size() > 1)
            LBTHROW(morphio::SomaError(err.ERROR_MULTIPLE_SOMATA(somata)));

        if(somata.size() == 0)
            LBERROR(err.WARNING_NO_SOMA_FOUND());
    }

    void raiseIfNoParent(const Sample& sample) {
        if(sample.parentId > -1 && samples.count(sample.parentId) == 0)
            LBTHROW(morphio::MissingParentError(err.ERROR_MISSING_PARENT(sample)));
    }

    /**
       A neurite which is not attached to the soma
    **/
    inline bool isOrphanNeurite(const Sample& sample) {
        return (sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA);
    }

    inline bool isRootSection(const Sample& sample) {
        return isOrphanNeurite(sample) ||
            (samples[sample.parentId].type == SECTION_SOMA &&
             sample.type != SECTION_SOMA); // Exclude soma bifurcations
    }

    inline bool isSectionStart(const Sample& sample) {
        return (isRootSection(sample) ||
                isSectionEnd(samples[sample.parentId])); // Standard section
    }

    inline bool isSectionEnd(const Sample& sample) {
        return sample.id == lastSomaPoint || // End of soma
            children[sample.id].size() == 0 || // Reached leaf
            (children[sample.id].size() >= 2 && // Reached neurite bifurcation
             sample.type != SECTION_SOMA);
    }

    template <typename T> void appendSample(std::shared_ptr<T> somaOrSection,
                                            const Sample& sample) {
        somaOrSection->points().push_back(sample.point);
        somaOrSection->diameters().push_back(sample.diameter);
    }

    void _pushChildren(std::vector<int32_t>& vec, int32_t id) {
        for(auto childId: children[id]) {
            vec.push_back(childId);
            _pushChildren(vec, childId);
        }
    }

    void raiseIfNonConform(const Sample &sample) {
        raiseIfSelfParent(sample);
        raiseIfBrokenSoma(sample);
        raiseIfNoParent(sample);
        raiseIfDisconnectedNeurite(sample);
    }

    SomaType somaType() {
        switch(morph.soma()->points().size()) {
        case 0:
        {
            return SOMA_UNDEFINED;
        }
        case 1:
        {
            return SOMA_SINGLE_POINT;
        }

        // NeuroMorpho format is characterized by a 3 points soma
        // with a bifurcation at soma root
        case 3:
        {
            uint32_t somaRootId = children[-1][0];
            auto &somaChildren = children[somaRootId];
            uint32_t nSomaChildren = std::count_if(
                somaChildren.begin(), somaChildren.end(),
                [this](uint32_t id){return this->samples[id].type == SECTION_AXON;});

            if(nSomaChildren == 2) {
                LBERROR("Using neuromorpho 3-Point soma");
                //  NeuroMorpho is the main provider of morphologies, but they
                //  with SWC as their default file format: they convert all
                //  uploads to SWC.  In the process of conversion, they turn all
                //  somas into their custom 'Three-point soma representation':
                //   http://neuromorpho.org/SomaFormat.html
                return SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
            }
            return SOMA_CYLINDERS;
        }
        default:
            return SOMA_CYLINDERS;
        }
    }

    Property::Properties _buildProperties() {
        std::vector<int32_t> depthFirstSamples;
        _pushChildren(depthFirstSamples, -1);
        for(const auto id: depthFirstSamples) {
            const Sample& sample = samples[id];
            if(isSectionStart(sample))
                _processSectionStart(sample);
            else if(sample.type != SECTION_SOMA)
                swcIdToSectionId[sample.id] = swcIdToSectionId[sample.parentId];

            if(sample.type == SECTION_SOMA)
                appendSample(morph.soma(), sample);
            else {
                appendSample(morph.section(swcIdToSectionId.at(sample.id)), sample);
            }
        }

        Property::Properties properties = morph.buildReadOnly();
        properties._cellLevel._somaType = somaType();
        return properties;
    }

    /**
       - Append last point of previous section if current section is not a root section
       - Update the parent ID of the new section
    **/
    void _processSectionStart(const Sample& sample) {
        int id = isRootSection(sample) ? -1 : swcIdToSectionId[sample.parentId];
        Property::PointLevel properties;

        if(!isRootSection(sample)) // Duplicating last point of previous section
        {
            properties._points.push_back(samples[sample.parentId].point);
            properties._diameters.push_back(samples[sample.parentId].diameter);
        }

        swcIdToSectionId[sample.id] = morph.appendSection(
            id,
            sample.type,
            properties);
    }

private:
    // Dictionnary: SWC Id of the last point of a section to morphio::mut::Section ID
    std::map<uint32_t, uint32_t> swcIdToSectionId;
    int lastSomaPoint = -1;
    std::map<int32_t, std::vector<uint32_t>> children;
    std::map<uint32_t, Sample> samples;
    int currentSectionParentId = -1;
    mut::Morphology morph;
    std::string uri;
    ErrorMessages err;

};




Property::Properties load(const URI& uri)
{

    auto properties = SWCBuilder(uri)._buildProperties();
    properties._cellLevel._cellFamily = FAMILY_NEURON;
    properties._cellLevel._version = MORPHOLOGY_VERSION_SWC_1;
    return properties;
}

} // namespace swc
} // namespace plugin
} // namespace morphio
