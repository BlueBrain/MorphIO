#ifndef X3D_EXPORTER_H
#define X3D_EXPORTER_H


#include <string>
#include <fstream>
#include <functional>

#include <morpho/morpho_h5_v1.hpp>

namespace morpho{

class x3d_exporter
{
public:
    x3d_exporter(const std::string & morphology_filename, const std::string & mesh_filename);


    void export_to_sphere();

private:
    morpho::h5_v1::morpho_reader reader;
    std::ofstream x3d_stream;
    std::string dest_filename;


    void envelop_header_and_footer(const std::function<void (void)> & fcontent);

    void html_viewer();

    void export_all_points();
};


}

#endif // X3D_EXPORTER_H
