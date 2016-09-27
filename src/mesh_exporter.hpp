#ifndef MESH_EXPORTER_H
#define MESH_EXPORTER_H

#include <fstream>
#include <iostream>
#include <string>

#include <morpho/morpho_h5_v1.hpp>



namespace morpho{

class gmsh_exporter
{
public:
    gmsh_exporter(const std::string & morphology_filename, const std::string & mesh_filename);


    void export_to_point_cloud();



private:
    std::ofstream geo_stream;
    morpho::h5_v1::morpho_reader reader;

    void serialize_header();

    void serialize_points_raw();

};


} //morpho

#endif // MESH_EXPORTER_H
