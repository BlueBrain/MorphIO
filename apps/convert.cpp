#include <morphio/mut/morphology.h>

#include <iostream>
int main(int argc, char* argv[]) {
    if(argc < 3) {
        LBERROR("Usage: ./convert infile outfile"
            "\n\nNote: infile and outfile must end with one of the following extension:\n"
            "swc, asc, h5");
        exit(-1);
    }

    morphio::mut::Morphology morphology(argv[1]);

    std::string destination(argv[2]);
    const size_t pos = destination.find_last_of(".");
    if(pos == std::string::npos) {
        LBERROR("Cannot determine extension of: " + destination);
        exit(-1);
    }


    std::string extension;
    for(auto& c : destination.substr(pos))
        extension += std::tolower(c);

    if(extension == ".asc")
        morphology.write_asc(argv[2]);
    else if(extension == ".swc")
        morphology.write_swc(argv[2]);
    else if(extension == ".h5")
        morphology.write_h5(argv[2]);
    else {
        LBERROR("Unknown destination extension: " + destination);
    }


}
