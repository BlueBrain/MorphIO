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

namespace morphio
{
namespace plugin
{
namespace swc
{

enum ErrorLevel {
    INFO,
    WARNING,
    ERROR
};

// It's not clear if -1 is the only way of identifying a root section.
const int SWC_UNDEFINED_PARENT = -1;

struct Sample
{
    Sample() : valid(false) , type(SECTION_UNDEFINED) , parentId(-1), lineNumber(-1)
        {
        }

    explicit Sample(const char* line, int lineNumber) : lineNumber(lineNumber)
        {
            float radius;
            valid = sscanf(line, "%20d%20d%20f%20f%20f%20f%20d", (int*)&id, (int*)&type, &point[0],
                           &point[1], &point[2], &radius, &parentId) == 7;

            diameter = radius * 2; // The point array stores diameters.

            if (type >= SECTION_CUSTOM_START)
                valid = false; // Unknown section type, custom samples are also
            // Regarded as unknown.
        }

    float diameter;
    bool valid;
    Point point; // x, y, z and diameter
    SectionType type;
    int parentId;
    int id;
    int lineNumber;
};

std::string errorLink(const std::string& filename, int lineNumber, ErrorLevel errorLevel) {
    std::map<ErrorLevel, int> color{
        {ErrorLevel::INFO, 34},
        {ErrorLevel::WARNING, 33},
        {ErrorLevel::ERROR, 31}};

    std::map<ErrorLevel, std::string> severity{
        {ErrorLevel::INFO, "info"},
        {ErrorLevel::WARNING, "warning"},
        {ErrorLevel::ERROR, "error"}};


    std::cout << "errorLevel: " << errorLevel << std::endl;
    std::cout << "color[errorLevel]: " << color[errorLevel] << std::endl;
    return "\e[1;" + std::to_string(color[errorLevel]) + "m" + filename + ":" + std::to_string(lineNumber) + ":" + severity[errorLevel] + "\e[0m";
}

std::string errorMsg(const std::string& filename,
                     int lineNumber,
                     ErrorLevel errorLevel,
                     std::string additionalMessage = ""){
    return "\n" + errorLink(filename, lineNumber, errorLevel) + additionalMessage;
}

const std::map<uint32_t, Sample> _readSamples(const URI& uri)
{
    std::ifstream file(uri.c_str());
    if (file.fail())
        LBTHROW(morphio::RawDataError("Error opening morphology file: " +
                                      uri));

    std::map<uint32_t, Sample> samples;

    size_t lineNumber = 0;
    std::string line;
    while (!std::getline(file, line).fail())
    {
        ++lineNumber;
        if (std::regex_match (line, std::regex(" *(#.*)?") ) || line.empty())
            continue;

        const auto &sample = Sample(line.data(), lineNumber);
        if (!sample.valid)
        {
            LBTHROW(morphio::RawDataError(errorMsg(uri, lineNumber, ErrorLevel::ERROR,
                                              "\nUnable to parse this line\n")));
        }
        samples[sample.id] = sample;
    }
    return samples;
}


/**
   Parsing SWC according to this specification:
   http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
**/
class SWCBuilder
{
public:
    SWCBuilder(const std::string& uri) : uri(uri) {
        samples = _readSamples(uri);
        lastSomaPoint = -1;
        for(auto sample_pair: samples){
            const auto &sample = sample_pair.second;
            children[sample.parentId].push_back(sample.id);
            if(sample.type == SECTION_SOMA)
                lastSomaPoint = sample.id;
            raiseIfNoParent(sample);
            raiseIfDisconnectedNeurite(sample);
        }

        checkSoma();
    }

    /**
       Are considered potential somata all sample
       with parentId == -1 and sample.type == SECTION_SOMA
     **/
    std::vector<Sample> _potentialSomata() {
        std::vector<Sample> somata;
        for(auto id: children[-1])
            if(samples[id].type == SECTION_SOMA)
                somata.push_back(samples[id]);
        return somata;
    }

    void raiseIfDisconnectedNeurite(const Sample& sample) {
        if(sample.parentId == SWC_UNDEFINED_PARENT && sample.type != SECTION_SOMA)
            LBERROR(errorMsg(uri, sample.lineNumber, ErrorLevel::WARNING,
                             "\nFound a disconnected neurite.\n"
                             "Neurites are not supposed to have parentId: -1\n"
                             "(although this is normal if this neuron has no soma)"));
    }

    void checkSoma() {
        auto somata = _potentialSomata();

        if(somata.size() > 1) {
            std::string links;
            for(auto soma: somata)
                links += "\n" + errorMsg(uri, soma.lineNumber, ErrorLevel::ERROR);
            LBTHROW(morphio::SomaError("\nMultiple somata found:" + links));
        }

        if(somata.size() == 0)
            LBERROR(errorMsg(uri, 0, ErrorLevel::WARNING, "\nNo soma found in file"));
    }

    void raiseIfNoParent(const Sample& sample) {
        if(sample.parentId > -1 && samples.count(sample.parentId) == 0)
                       LBTHROW(morphio::MissingParentError(
                                   errorMsg(uri, sample.lineNumber, ErrorLevel::ERROR,
                                 "\nSample id: " + std::to_string(sample.id) +
                                 " refers to non-existant parent ID: " +
                                 std::to_string(sample.parentId))));
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

    bool isSectionStart(const Sample& sample) {
        return (isRootSection(sample) ||
                isSectionEnd(samples[sample.parentId])); // Standard section
    }

    bool isSectionEnd(const Sample& sample) {
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

        return morph.buildReadOnly();
    }

    /**
       - Append last point of previous section if current section is not a root section
       - Update the parent ID of the new section
    **/
    void _processSectionStart(const Sample& sample) {
        int id = isRootSection(sample) ? -1 : swcIdToSectionId[sample.parentId];
        swcIdToSectionId[sample.id] = morph.appendSection(
            id,
            sample.type,
            Property::PointLevel());

        if(!isRootSection(sample)) // Duplicating last point of previous section
        {
            appendSample(morph.section(swcIdToSectionId[sample.id]), samples[sample.parentId]);
        }
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
