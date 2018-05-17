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
#include <morphio/mut/mito_iterators.h>
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
    morphio::mut::Morphology morpho;
    morpho.soma()->points() = {{0, 0, 0}, {1, 1, 1}};
    morpho.soma()->diameters() = {1, 1};

    auto mito = morpho.mitochondria();

    uint32_t sectionId = morpho.appendSection(
        -1, morphio::Property::PointLevel({{2, 2, 2}, {3, 3, 3}}, {4, 4}, {5, 5}),
        morphio::SectionType::SECTION_AXON);

    uint32_t id = mito.appendSection(
        -1, morphio::Property::MitochondriaPointLevel({0, 0}, {0.5, 0.6},
                                                      {10, 20}));

    mito.appendSection(
        id, morphio::Property::MitochondriaPointLevel({0, 0, 0, 0},
                                                      {0.6, 0.7, 0.8, 0.9},
                                                      {20, 30, 40, 50}));


    auto roots = mito.rootSections();
    auto first_root = roots[0];

    for(auto id_it = mito.depth_begin(first_root); id_it != mito.depth_end(); ++id_it) {
        int section_id = *id_it;
        auto& section = mito.section(section_id);
        for(int i = 0; i<section->diameters().size(); ++i) {
            std::cout << "section->diameters()[i]: " << section->diameters()[i] << std::endl;
        }
    }
}
