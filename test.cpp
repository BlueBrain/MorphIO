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

// cd /home/bcoste/workspace/morphology/io/build && make && cd .. && rm main; g++ -g -std=c++1z -I ./include -I 3rdparty/glm/ -I 3rdparty/HighFive -I 3rdparty/GSL_LITE/include -L build/src/ test.cpp -o main  -lmorphio -Wl,-rpath,/usr/lib/x86_64-linux-gnu/hdf5/serial /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so && LD_LIBRARY_PATH=./build/src ./main && pip install . --upgrade

#define go_mut 1
int main()
{
    std::cout << "bla" << std::endl;
    #ifdef go_mut
    morphio::mut::Morphology m("/home/bcoste/workspace/morphology/io/tests/data/simple.swc");
    #else
    morphio::Morphology m("/home/bcoste/workspace/morphology/io/tests/data/simple.swc");
    #endif

    auto roots = m.rootSections();

    auto first_root = roots[0];
    #ifdef go_mut
    for(auto it = first_root->depth_begin(); it!=first_root->depth_end(); ++it)
    #else
    for(auto it = first_root.depth_begin(); it!=first_root.depth_end(); ++it)
    #endif
        std::cout << "(*it): " << (*it) << std::endl;
}
