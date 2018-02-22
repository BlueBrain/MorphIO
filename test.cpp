#include <array>
#include <iostream>
#include <vector>

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/sectionBuilder.h>
#include <morphio/soma.h>

using namespace std;

// To be compiled and run with:
// g++ -std=c++1z -I ./include -I 3rdparty/glm/ -I 3rdparty/HighFive _I
// 3rdparty/GSL/include -L build/src/ test.cpp -o main  -lmorphio
// -Wl,-rpath,/usr/lib/x86_64-linux-gnu/hdf5/serial
// /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so &&
// LD_LIBRARY_PATH=./build/src ./main

int main()
{
    morphio::Morphology morphology("neuron.swc");
    for (auto type : morphology.sectionTypes())
        std::cout << "type: " << type << std::endl;

    auto family = morphology.cellFamily();
    std::cout << "family: " << family << std::endl;
    auto sections = morphology.sections();
    std::cout << "sections.size(): " << sections.size() << std::endl;
    auto section = sections[3];
    std::cout << "depth" << std::endl;
    for (auto it = section.depth_begin(); it != section.depth_end(); ++it)
    {
        std::cout << "section.id(): " << (*it).id() << std::endl;
    }

    std::cout << "breadth" << std::endl;
    for (auto it = section.breadth_begin(); it != section.breadth_end(); ++it)
    {
        std::cout << "section.type(): " << (*it).type() << std::endl;
    }

    std::cout << "upstream" << std::endl;
    section = morphology.section(3);
    for (auto it = section.upstream_begin(); it != section.upstream_end(); ++it)
    {
        std::cout << "section.id(): " << (*it).id() << std::endl;
    }

    auto soma = morphology.soma();

    for (auto point : soma.points())
        std::cout << point[0] << ", " << point[1] << ", " << point[2]
                  << std::endl;

    auto center = soma.somaCenter();
    std::cout << center[0] << ", " << center[1] << ", " << center[2]
              << std::endl;

    std::cout << "Root sections from morphology: " << std::endl;
    for (auto section : morphology.rootSections())
        std::cout << section.id() << std::endl;

    morphio::builder::Morphology a(morphology);
    for (auto section : a.rootSections())
    {
        std::cout << "Section" << std::endl;
        for (auto& child : section->children())
        {
            std::cout << "child->id(): " << child->id() << std::endl;
        }
    }

    std::cout << "Traversal" << std::endl;
    auto firstNeurite = *(a.rootSections().begin());
    a.traverse(
        [](morphio::builder::Morphology* morph,
           morphio::builder::Section* sec) {
            std::cout << "hello from: " << sec->type() << std::endl;
            for (auto center : sec->points())
                std::cout << center[0] << ", " << center[1] << ", " << center[2]
                          << std::endl;

        },
        firstNeurite);

    std::cout << "H5 Writer" << std::endl;
    morphio::builder::writer::h5(a);

    std::cout << "SWC writer" << std::endl;
    morphio::builder::writer::swc(a);

    std::cout << "Asc writer" << std::endl;
    morphio::builder::writer::asc(a);

    std::cout << "End" << std::endl;
}
