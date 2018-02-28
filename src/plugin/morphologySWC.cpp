#include <cassert>
#include <fstream>
#include <list>
#include <sstream>

#include <iostream>

#include <morphio/properties.h>
#include "morphologySWC.h"
#include <morphio/mut/morphology.h>
#include <morphio/mut/soma.h>

namespace morphio
{
namespace plugin
{
namespace swc
{
// It's not clear if -1 is the only way of identifying a root section.
const int SWC_UNDEFINED_PARENT = -1;

struct Sample
{
    Sample() : valid(false) , type(SECTION_UNDEFINED) , parentId(-1)
        {
        }

    explicit Sample(const char* line)
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
};


std::string errorMsg(const std::string& filename,
                     const std::string& line,
                     int lineNumber,
                     std::string additionalMessage = ""){
    return "Error reading swc morphology file: " + filename +
        "\nParse error at line: " + std::to_string(lineNumber) +
        "\nParsed line was:" + line + "\n\n" +
        additionalMessage;
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
        if (line[0] == '#' || line.empty())
            continue;

        const auto &sample = Sample(line.data());
        if (!sample.valid)
        {
            LBTHROW(morphio::RawDataError(errorMsg(uri, line, lineNumber)));
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
    SWCBuilder(const std::map<uint32_t, Sample>& _samples) : samples(_samples)
        {
            lastSomaPoint = -1;
            for(auto sample_pair: samples){
                const auto &sample = sample_pair.second;
                children[sample.parentId].push_back(sample.id);
                if(sample.type == SECTION_SOMA)
                    lastSomaPoint = sample.id;
            }
        }


    bool isSectionStartButSoma(const Sample& sample) {
        return properties._points.size() == 0 && // Start of a new section
            sample.parentId != SWC_UNDEFINED_PARENT; // filter out soma section
    }

    bool isSectionEnd(const Sample& sample) {
            return sample.id == lastSomaPoint || // End of soma
                children[sample.id].size() == 0 || // Reached leaf
                children[sample.id].size() >= 2; // Reached bifurcation
        }

    void appendSample(const Sample& sample) {
        properties._points.push_back(sample.point);
        properties._diameters.push_back(sample.diameter);
    }

    Property::Properties _buildProperties()
        {
            for(auto sample_pair: samples) {
                const auto &sample = sample_pair.second;

                if(isSectionStartButSoma(sample))
                    _processSectionStart(sample);

                appendSample(sample);

                if(isSectionEnd(sample))
                    _processSectionEnd(sample);
            }

            return morph.buildReadOnly();
        }

    /**
       - Append last point of previous section if current section is not a root section
       - Update the parent ID of the new section
    **/
    void _processSectionStart(const Sample& sample)
        {
            bool isRootSection = samples.at(sample.parentId).type == SECTION_SOMA;

            if(!isRootSection)
            {
                appendSample(samples.at(sample.parentId));
                currentSectionParentId = swcIdToSectionId.at(sample.parentId);
            } else {
                currentSectionParentId = -1;
            }
        }

/**
   1) Register in mut::Morphology the Soma/Section with the property object
   2) Reset the property object
**/
    void _processSectionEnd(const Sample& sample)
        {
            if(sample.id == lastSomaPoint){
                morph.soma() = std::make_shared<mut::Soma>(mut::Soma(properties));
            } else {
                swcIdToSectionId[sample.id] = morph.appendSection(currentSectionParentId,
                                                                  sample.type,
                                                                  properties);
            }
            properties = Property::PointLevel();
        }

private:
    // Dictionnary: SWC Id of the last point of a section to morphio::mut::Section ID
    std::map<uint32_t, uint32_t> swcIdToSectionId;
    int lastSomaPoint = -1;
    std::map<uint32_t, std::vector<uint32_t>> children;
    std::map<uint32_t, Sample> samples;
    Property::PointLevel properties;
    int currentSectionParentId = -1;
    mut::Morphology morph;

};

Property::Properties load(const URI& uri)
{

    return SWCBuilder(_readSamples(uri))._buildProperties();
}

} // namespace swc
} // namespace plugin
} // namespace morphio
