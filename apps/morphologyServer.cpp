
/*
 * Copyright (c) 2017, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include <brion/constants.h>
#include <brion/morphology.h>
#include <keyv/Map.h>
#include <lunchbox/daemon.h>
#include <lunchbox/log.h>
#include <zeroeq/server.h>
#include <zeroeq/uri.h>

int main(const int argc, char* argv[])
{
    zeroeq::Server server(argc == 2 ? zeroeq::URI(argv[1]) : zeroeq::URI());
    const std::string address = server.getURI().getHost() + ":" +
                                std::to_string(int(server.getURI().getPort()));
    std::cout
        << "Morphology server reachable on:" << std::endl
        << "  export ZEROEQ_SERVERS=\"" << address << ",$ZEROEQ_SERVERS\""
        << std::endl
        << "  zeroeq://" << address << "/path/to/morphology" << std::endl
        << std::endl
        << "  [c]ache read, [d]isk read with cache update, disk read "
           "with cache [e]rror, [u]ncached disk read, morphology [l]oad error: "
        << std::flush;
    lunchbox::Log::setOutput(std::string(argv[0]) + ".log");

    auto cache = keyv::Map::createCache();

    server.handle(brion::ZEROEQ_GET_MORPHOLOGY, [&](const void* data,
                                                    const size_t size) {
        if (!data || !size)
            return zeroeq::ReplyData();

        const std::string path((const char*)data, size);
        if (cache)
        {
            servus::Serializable::Data value;
            cache->takeValues({path},
                              [&](const std::string&, char* d, const size_t s) {
                                  value.ptr.reset(d);
                                  value.size = s;
                              });

            if (value.ptr && value.size)
            {
                std::cout << 'c' << std::flush;
                return zeroeq::ReplyData(brion::ZEROEQ_GET_MORPHOLOGY, value);
            }
        }
        try
        {
            const brion::Morphology morphology{brion::URI(path)};
            servus::Serializable::Data bin = morphology.toBinary();
            if (cache)
            {
                if (cache->insert(path, bin.ptr.get(), bin.size))
                    std::cout << 'd' << std::flush;
                else
                    std::cout << 'e' << std::flush;
            }
            else
                std::cout << 'u' << std::flush;

            return zeroeq::ReplyData(brion::ZEROEQ_GET_MORPHOLOGY,
                                     morphology.toBinary().clone());
        }
        catch (const std::exception& e)
        {
            std::cout << 'l' << std::flush;
            LBWARN << "Failed to load " << path << ": " << e.what()
                   << std::endl;
            return zeroeq::ReplyData();
        }
    });

    while (true)
        server.receive();
}
