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
    return morphio::Morphology("/home/bcoste/workspace/morphology/toolbox/morph_tool/tests/simple.asc") == morphio::Morphology("/home/bcoste/workspace/morphology/toolbox/morph_tool/tests/blah.swc");
}
