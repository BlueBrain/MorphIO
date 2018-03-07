#pragma once

#include <morphio/properties.h>
#include <morphio/types.h>

namespace morphio
{
namespace plugin
{
namespace swc
{
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

Property::Properties load(const URI& uri);
} // namespace swc

} // namespace plugin

} // namespace morphio
