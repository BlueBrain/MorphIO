#pragma once

#include <string>

#include <minimorph/api.h>
#include <minimorph/properties.h>
#include <minimorph/morphology.h>
#include <minimorph/types.h>

namespace minimorph
{
/**
 * Basic plugin init data for MorphologyPlugin.
 * @version 1.4
 */
class MorphologyInitData
{
public:
    explicit MorphologyInitData(
        const URI& uri, const MorphologyVersion v = MORPHOLOGY_VERSION_H5_1_1,
        const CellFamily f = FAMILY_NEURON)
        : uri(uri)
        , version(v)
        , family(f)
    {
        if (f == FAMILY_GLIA && v != MORPHOLOGY_VERSION_H5_1_1)
            throw std::runtime_error(
                "Glia cells only support HDF5 version 1.1");
    }

    MorphologyInitData(const URI& uri, const MorphologyVersion v,
                       const unsigned int accessMode)
        : uri(uri)
        , version(v)
        , family(FAMILY_NEURON)
    {
    }

    MorphologyInitData(const URI& uri, const CellFamily f)
        : uri(uri)
        , version(MORPHOLOGY_VERSION_H5_1_1)
        , family(f)
    {
    }

    URI getURI() const{ return URI(uri); }

    MorphologyVersion version;
    CellFamily family;
    URI uri;
};

/**
 * Base interface for morphology readers plugins.
 *
 * The following example creates a new plugin and registers it:
 * @code
 * class MyMorphology : MorphologyPlugin
 * {
 *     MyMorphology( const MorphologyInitData& initData );
 *     static bool handles( const MorphologyInitData& initData );
 *     ...
 * };
 * ...
 * // in the .cpp file
 * namespace
 * {
 *     PluginRegisterer< MyMorphology > registerer;
 * }
 * @endcode
 *
 * @version 1.4
 */
class MorphologyPlugin
{
public:
    MorphologyPlugin(const MorphologyInitData& data)
        : _data(data)
    {
    }


    void extractInformation()
    {
        const auto& sections = _properties.get<Property::Section>();
        auto& children = _properties.get<Property::Children>();

        for (size_t i = 0; i < sections.size(); ++i)
        {
            const int32_t parent = sections[i][1];
            if (parent != -1)
                children[parent].push_back(i);
        }

        _properties.get<Property::CellFamily>() = _data.family;
    }


    virtual ~MorphologyPlugin() {}

    /** Load all data of the morphology.
     *  Needs to be thread-safe wrt other instances. May throw std::exception.
     */
    virtual void load() = 0;

    /** @internal */
    MorphologyInitData& getInitData() { return _data; }
    const MorphologyInitData& getInitData() const { return _data; }

    /** @internal */
    MorphologyVersion getVersion() const { return _data.version; }

    const Property::Properties getProperties() const { return _properties; }
protected:
    MorphologyInitData _data;
    Property::Properties _properties;

    /*
    // Serializable API
    std::string getTypeName() const final { return "minimorph::MorphologyPlugin"; }
    bool _fromBinary(const void* data, const size_t size) final;
    */
};

}
