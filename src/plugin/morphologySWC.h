
#ifndef BRAIN_PLUGIN_MORPHOLOGYSWC
#define BRAIN_PLUGIN_MORPHOLOGYSWC

#include <minimorph/types.h>

#include "../morphologyPlugin.h"
/* #include "../vector_types.h" */

namespace minimorph
{
namespace plugin
{
class MorphologySWC : public MorphologyPlugin
{
public:
    explicit MorphologySWC(const MorphologyInitData& initData);

    /** Check if this plugin can handle the given uri. */
    static bool handles(const MorphologyInitData& initData);
    static std::string getDescription();

private:
    // Plugin API
    void load() final;

    struct RawSWCInfo;
    void _readSamples(RawSWCInfo& info);
    void _buildSampleTree(RawSWCInfo& info);
    void _buildStructure(RawSWCInfo& info);
};
}
}
#endif
