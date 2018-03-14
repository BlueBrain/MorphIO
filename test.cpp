#include <cassert>
#include <array>
#include <iostream>
#include <vector>

#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/writers.h>


using namespace std;

// To be compiled and run with:

//cd /home/bcoste/workspace/morphology/io/build && make && cd .. && rm main; g++ -g -std=c++1z -I ./include -I 3rdparty/glm/ -I 3rdparty/HighFive -I 3rdparty/GSL/include -L build/src/ test.cpp -o main  -lmorphio -Wl,-rpath,/usr/lib/x86_64-linux-gnu/hdf5/serial /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so && LD_LIBRARY_PATH=./build/src ./main && pip install . --upgrade
void testChildren(){
    morphio::Morphology morphology("/home/bcoste/workspace/morphology/neurom/test_data/swc/simple.swc");
    int nChildren = morphology.sections()[1].children().size();
    // for(auto id: morphology.sections()[1].children()){
    //     std::cout << "morphology.section(id).points(): " << morphology.section(id).points() << std::endl;
    // }
    std::cout << "nChildren: " << nChildren << std::endl;
    assert(nChildren == 2 && "Should have 2 children");
}
int main()
{
    // std::string filename("Neuron.h5");
    // morphio::Morphology morphology(filename);
    // morphio::mut::Morphology a(morphology);

    // testChildren();

    // for (auto type : morphology.sectionTypes())
    //     std::cout << "type: " << type << std::endl;

    // auto family = morphology.cellFamily();
    // std::cout << "family: " << family << std::endl;
    // auto sections = morphology.sections();
    // std::cout << "sections.size(): " << sections.size() << std::endl;
    // auto section = sections[3];
    // std::cout << "depth" << std::endl;
    // for (auto it = section.depth_begin(); it != section.depth_end(); ++it)
    // {
    //     std::cout << "section.id(): " << (*it).id() << std::endl;
    // }

    // // std::cout << "morphology.rootSections()[1]: " << morphology.rootSections()[1] << std::endl;
    // // for(auto sec: morphology.rootSections()[0].children())
    // // {
    // //     std::cout << sec << std::endl;
    // // }

    // // auto section = morphology.rootSections()[0];
    // // std::cout << "breadth" << std::endl;
    // // for (auto it = section.breadth_begin(); it != section.breadth_end(); ++it)
    // // {
    // //     std::cout << (*it) << std::endl;
    // // }

    // // std::cout << "upstream" << std::endl;
    // // section = morphology.section(3);
    // // for (auto it = section.upstream_begin(); it != section.upstream_end(); ++it)
    // // {
    // //     std::cout << "section.id(): " << (*it).id() << std::endl;
    // // }

    // // auto soma = morphology.soma();

    // // for (auto point : soma.points())
    // //     std::cout << point[0] << ", " << point[1] << ", " << point[2]
    // //               << std::endl;

    // // auto center = soma.somaCenter();
    // // std::cout << center[0] << ", " << center[1] << ", " << center[2]
    // //           << std::endl;

    // // std::cout << "Root sections from morphology: " << std::endl;
    // // for (auto section : morphology.rootSections())
    // //     std::cout << section.id() << std::endl;

    // // for (auto section : a.rootSections())
    // // {
    // //     std::cout << "Section" << std::endl;
    // //     for (auto& child : a.children(section))
    // //     {
    // //         std::cout << "child->id(): " << child << std::endl;
    // //     }
    // // }

    // // std::cout << "Traversal" << std::endl;
    // // auto firstNeurite = *(a.rootSections().begin());
    // // a.traverse(
    // //     [](morphio::mut::Morphology& morph, int id) {
    // //         std::cout << morph.section(id) << std::endl;
    // //     },
    // //     firstNeurite);

    // // std::cout << "H5 Writer" << std::endl;
    // // morphio::mut::writer::h5(a);

    // // std::cout << "SWC writer" << std::endl;
    // // morphio::mut::writer::swc(a);

    // // std::cout << "Asc writer" << std::endl;
    // // morphio::mut::writer::asc(a);

    // std::cout << "End" << std::endl;

    // morphio::mut::Morphology m;
    // uint32_t id = m.appendSection(-1,
    //                               morphio::SECTION_AXON,
    //                               morphio::Property::PointLevel({{1, 2, 3}, {4, 5, 6}},
    //                                                             {2, 2},
    //                                                             {20, 20}));


    // m.appendSection(id,
    //                 morphio::SECTION_AXON,
    //                 morphio::Property::PointLevel({{400, 5, 6}, {7, 8, 9}},
    //                            {2, 3},
    //                                               {20, 30}));

    // m.buildReadOnly();


}
