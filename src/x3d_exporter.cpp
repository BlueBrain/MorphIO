#include "x3d_exporter.hpp"

#include <hadoken/format/format.hpp>
#include <boost/filesystem/path.hpp>

#include <morpho/morpho_tree.hpp>

namespace fmt = hadoken::format;
namespace fs = boost::filesystem;

namespace morpho{


void points_distance_to_sphere(const branch::point & point, double distance,
                               std::ostream & output){

    fmt::scat(output,
    "<Transform translation='", point[0], " ", point[1], " ", point[2], "'>" "\n"
    "  <Shape>"  "\n"
    "    <Sphere radius='", distance/2, "' />"  "\n"
    "    <Appearance>"  "\n"
    "      <Material DEF='MaterialLightBlue' diffuseColor='0.1 0.5 1'/>"  "\n"
    "    </Appearance>"  "\n"
    "  </Shape>"  "\n"
    "</Transform>"  "\n"

    );
}


x3d_exporter::x3d_exporter(const std::string & morphology_filename, const std::string & x3d_filename) :
    reader(morphology_filename),
    x3d_stream(x3d_filename)
{

}

void x3d_exporter::export_to_sphere(){
    envelop_header_and_footer([this]{
        this->export_all_points();
    });
}


void x3d_exporter::export_all_points(){
    fmt::scat(x3d_stream, "    <Group>" "\n");

    morpho_tree tree = reader.create_morpho_tree(morpho::h5_v1::morpho_reader::generate_single_soma);

    for(std::size_t b_id =0; b_id < tree.get_tree_size(); ++b_id){

        auto & branch = tree.get_branch(b_id);
        auto & points = branch.get_points();
        auto & distance = branch.get_distances();

        assert(points.size1() == distance.size());
        for(std::size_t i = 0; i < points.size1(); ++i){
            points_distance_to_sphere({ points(i,0), points(i,1), points(i,2) }, distance[i], x3d_stream);
        }
    }

    fmt::scat(x3d_stream, "    </Group>" "\n");
}


void x3d_exporter::envelop_header_and_footer(const std::function<void ()> & fcontent){

    fmt::scat(x3d_stream, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" "\n"
            "<!DOCTYPE X3D PUBLIC \"ISO//Web3D//DTD X3D 3.3//EN\" \"http://www.web3d.org/specifications/x3d-3.3.dtd\">" "\n"
            "<X3D profile='Immersive' version='3.3' xmlns:xsd='http://www.w3.org/2001/XMLSchema-instance' xsd:noNamespaceSchemaLocation='http://www.web3d.org/specifications/x3d-3.3.xsd'>" "\n"
            "  <head>" "\n"
            "  </head>" "\n"
            "  <Scene>" "\n"
            "<!-- Scene generated with morpho-tool from morphology", reader.get_filename() ," -->" "\n"
            " <WorldInfo title='", fs::path(reader.get_filename()).filename(),"'/>" "\n");

    fcontent();
    fmt::scat(x3d_stream,
                          "  </Scene>" "\n""\n"
                          "</X3D>" "\n"
              );



}

} // morpho
