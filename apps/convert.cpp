#include <morphio/mut/morphology.h>

#include <cmath>
#include <iostream>

std::string getExtension(std::string filename) {
    const size_t pos = filename.find_last_of(".");
    if(pos == std::string::npos) {
        LBERROR("Cannot determine extension of: " + filename);
        exit(-1);
    }

    std::string extension;
    for(auto& c : filename.substr(pos+1))
        extension += std::tolower(c);

    return extension;
}

/**
   Transform a cylindrical representation of the soma into
   a H5 contour. The contour is a circle in the XY plane
   having the same surface than the original cylinder
**/
void soma_swc_to_h5(morphio::mut::Morphology& morpho) {
    std::cout << "morpho.somaType(): " << morpho.somaType() << std::endl;
    if(morpho.soma()->points().empty())
        return;

    float swc_cylinder_radius = morpho.soma()->diameters()[0] / 2;
    float z_mean = morpho.soma()->points()[0][2];

    int n_points = 40;
    std::vector<morphio::Point> points;
    std::vector<float> diameters;

    float h5_contour_radius = swc_cylinder_radius;

    for(int i = 0;i<n_points; ++i){
        float x =  h5_contour_radius * cos(float(2*M_PI) * i / float(n_points));
        float y = h5_contour_radius * sin(float(2*M_PI) * i / float(n_points));
        float z = z_mean;
        morphio::Point point{x,y,z};
        points.push_back(point);
        diameters.push_back(0);
    }

    morpho.soma()->points() = points;
    morpho.soma()->diameters() = diameters;
}


int main(int argc, char* argv[]) {
    if(argc < 3) {
        LBERROR("Usage: ./convert infile outfile"
            "\n\nNote: infile and outfile must end with one of the following extension:\n"
            "swc, asc, h5");
        exit(-1);
    }

    morphio::mut::Morphology morphology(argv[1]);

    std::string source(argv[1]);
    std::string destination(argv[2]);

    auto source_extension = getExtension(source);
    auto destination_extension = getExtension(destination);


    if(destination_extension == "asc")
        morphology.write_asc(argv[2]);
    else if(destination_extension == "swc")
        morphology.write_swc(argv[2]);
    else if(source_extension == "swc" && destination_extension == "h5") {
        soma_swc_to_h5(morphology);
        morphology.write_h5(argv[2]);
    } else {
        LBERROR("Unknown destination extension: " + destination_extension);
    }
}
