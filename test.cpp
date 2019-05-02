#include <array>
#include <cassert>
#include <iostream>
#include <vector>

#include <morphio/mito_section.h>
#include <morphio/mitochondria.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
#include <morphio/vasc/section.h>
#include <morphio/vasc/properties.h>
#include <morphio/vasc/vasculature.h>

#include <morphio/mut/mitochondria.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/mito_iterators.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>

using namespace std;

// To be compiled and run with:

// cd /home/bcoste/workspace/morphology/io/build && make && cd .. && rm main; g++ -g -std=c++1z -I ./include -I 3rdparty/glm/ -I 3rdparty/HighFive -I 3rdparty/GSL_LITE/include -L build/src/ test.cpp -o main  -lmorphio -Wl,-rpath,/usr/lib/x86_64-linux-gnu/hdf5/serial /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so && LD_LIBRARY_PATH=./build/src ./main && pip install . --upgrade
int main()
{
    using namespace std;
    clock_t begin = clock();
    morphio::vasculature::Vasculature m("tests/data/h5/vasculature2.h5");
    morphio::vasculature::Vasculature m1("tests/data/h5/vasculature2.h5");
    //morphio::Morphology m("data/Astrocytes_H5/astrocyte2_v3.h5");
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC * 1000.;
    std::cout << elapsed_secs << " milliseconds" << std::endl;
    std::cout << (m == m1) << std::endl;
    //morphio::mut::Morphology b(m);
    //for(auto it = m.begin(); it != m.end(); ++it)
    //    std::cout <<  (*it).id() << std::endl;


    // m.sanitize();

    // std::cout << "second sanitize" << std::endl;
    // b.sanitize();

    // std::cout << "m.rootSections()[0]->children()[0]->parent().get(): " << m.rootSections()[0]->children()[0]->parent().get() << std::endl;
    //     (*it)->points() = {{0,0,0}, {0,0,0}};

    // for(auto it = m.depth_begin(); it != m.depth_end(); ++it)
    //     std::cout << "dumpPoints((*it)->points()): " << morphio::dumpPoints((*it)->points()) << std::endl;

    // child->appendSection(morphio::Property::PointLevel({{0, 5, 0}, {500,5,5}}, {10, 10}));
    // m.write("test.swc");
}
