#ifndef MESH_EXPORTER_H
#define MESH_EXPORTER_H

#include <fstream>
#include <iostream>
#include <string>
#include <bitset>

#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_tree.hpp>


namespace morpho{



class gmsh_exporter
{
public:

    enum exporter_flags {
        none = 0x00,
        point_soma = 0x01
    };

    gmsh_exporter(const std::string & morphology_filename, const std::string & mesh_filename);


    void export_to_point_cloud();

    void export_to_wireframe();



private:
    std::ofstream geo_stream;
    morpho::h5_v1::morpho_reader reader;


    void serialize_header();

    void serialize_points_raw();

    std::pair<std::size_t, std::size_t> serialize_branch_points(const branch& b, std::size_t & counter);

    void serialize_branch_lines(morpho_tree & tree,
                                size_t branch,
                                const std::vector<std::pair<std::size_t, std::size_t> > & vec,
                                size_t &counter, std::size_t & point_counter );

};


} //morpho

#endif // MESH_EXPORTER_H
