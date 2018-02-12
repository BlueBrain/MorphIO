#include <iostream>
#include <vector>
#include <array>

#include <minimorph/morphology.h>
#include <minimorph/section.h>
#include <minimorph/sectionBuilder.h>

using namespace std;

// To be compiled and run with:
// g++ -std=c++1z -I ./include -I 3rdparty/glm/ -I 3rdparty/HighFive _I 3rdparty/GSL/include -L build/src/ test.cpp -o main  -lmorphio -Wl,-rpath,/usr/lib/x86_64-linux-gnu/hdf5/serial /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so && LD_LIBRARY_PATH=./build/src ./main

int main(){
    minimorph::Morphology morphology("neuron.swc");
    auto family = morphology.getCellFamily();
    std::cout << "family: " << family << std::endl;
    auto sections = morphology.getSections();
    std::cout << "sections.size(): " << sections.size() << std::endl;
    auto section = sections[3];
    std::cout << "depth" << std::endl;
    for(auto it = section.depth_begin(); it != section.depth_end(); ++it){
        std::cout << "section.getID(): " << (*it).getID() << std::endl;
    }

    std::cout << "breadth" << std::endl;
    for(auto it = section.breadth_begin(); it != section.breadth_end(); ++it){
        std::cout << "section.getID(): " << (*it).getID() << std::endl;
    }

    std::cout << "upstream" << std::endl;
    section = morphology.getSection(21);
    // for(auto it = section.upstream_begin(); it != section.upstream_end(); ++it){
    //     std::cout << "section.getID(): " << (*it).getID() << std::endl;
    // }

    auto soma = morphology.getSoma();

    for(auto point: soma.getPoints())
        std::cout << point[0] << ", " << point[1] << ", " << point[2] << std::endl;

    auto center = soma.getSomaCenter();
    std::cout << center[0] << ", " << center[1] << ", " << center[2] << std::endl;

    std::cout << "soma.getType(): " << soma.getType() << std::endl;

    std::cout << "Root sections: " << std::endl;
    for(auto section: soma.getRootSections())
        std::cout << section.getID() << std::endl;

    std::cout << "Root sections from morphology: " << std::endl;
    for(auto section: morphology.getRootSections())
        std::cout << section.getID() << std::endl;

    minimorph::builder::Morphology a(morphology);
    for(auto section: a.getRootSections()){
        std::cout << "Section" << std::endl;
        for(auto &child: section->getChildren()){
            std::cout << "child->getID(): " << child->getID() << std::endl;
        }
    }

    // std::cout << "After deletion" << std::endl;
    // for(auto section: a.getRootSections())
    //     std::cout << "section->getID(): " << section->getID() << std::endl;
    std::cout << "End" << std::endl;
}
