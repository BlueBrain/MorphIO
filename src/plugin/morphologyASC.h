#include "parse_ll.cpp"

namespace minimorph
{

namespace plugin
{
class MorphologyASC : public MorphologyPlugin
{
public:
    explicit MorphologyASC(const MorphologyInitData& initData)
        : MorphologyPlugin(initData)
        {
            load();
        }

private:
    void load() final {
        plugin::NeurolucidaParser parser;
        std::ifstream ifs(_data.uri);
        std::string input((std::istreambuf_iterator<char>(ifs)),
                          (std::istreambuf_iterator<char>()));

        _properties = parser.parse(input);
        _data.family = FAMILY_NEURON;
    }
};

} // namespace plugin
} // namespace minimorph
