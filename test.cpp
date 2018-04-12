#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>

using namespace std;

// To be compiled and run with:

// cd /home/bcoste/workspace/morphology/io/build && make && cd .. && rm main;
// g++ -g -std=c++1z -I ./include -I 3rdparty/glm/ -I 3rdparty/HighFive -I
// 3rdparty/GSL/include -L build/src/ test.cpp -o main  -lmorphio
// -Wl,-rpath,/usr/lib/x86_64-linux-gnu/hdf5/serial
// /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so &&
// LD_LIBRARY_PATH=./build/src ./main && pip install . --upgrade

int main()
{
    // morphio::mut::Morphology morpho;
    // morpho.soma()->points() = {{0, 0, 0}, {1, 1, 1}};
    // morpho.soma()->diameters() = {1, 1};

    // uint32_t sectionId = morpho.appendSection(
    //     -1, morphio::SectionType::SECTION_AXON,
    //     morphio::Property::PointLevel({{2, 2, 2}, {3, 3, 3}}, {4, 4}, {5, 5}));

    // uint32_t id = morpho.mitochondria().appendSection(
    //     -1, morphio::Property::MitochondriaPointLevel({0, 0}, {0.5, 0.6},
    //                                                   {10, 20}));

    // morpho.mitochondria().appendSection(
    //     id, morphio::Property::MitochondriaPointLevel({0, 0, 0, 0},
    //                                                   {0.6, 0.7, 0.8, 0.9},
    //                                                   {20, 30, 40, 50}));
    // morpho.write_h5("test.h5");

    morphio::Morphology m("test.h5");

    std::cout << "m.mitochondria().rootSections().size(): " << m.mitochondria().rootSections().size() << std::endl;
    // for(auto s: m.mitochondria().section(0).diameters())
    //     std::cout << "s: " << s << std::endl;
    // std::cout << "second" << std::endl;

    // for(auto s: m.mitochondria().section(1).diameters())
    //     std::cout << "s: " << s << std::endl;

}
