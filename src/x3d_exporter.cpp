#include "x3d_exporter.hpp"

#include <hadoken/format/format.hpp>
#include <boost/filesystem/path.hpp>

#include <morpho/morpho_tree.hpp>

namespace fmt = hadoken::format;
namespace fs = boost::filesystem;

namespace morpho{



void points_distance_to_sphere(const branch::point & point, double distance,
                               const std::string & sphere_unit_name,
                               std::ostream & output){

    fmt::scat(output,
    "<Transform translation='", point[0], " ", point[1], " ", point[2],
            "' scale='", distance/2," ", distance/2 ," ",distance/2 ,"' >" "\n"
    "  <Shape USE=\"",sphere_unit_name,"\" />"  "\n"
    "</Transform>"  "\n"

    );
}


x3d_exporter::x3d_exporter(const std::string & morphology_filename, const std::string & x3d_filename) :
    reader(morphology_filename),
    x3d_stream(x3d_filename),
    dest_filename(x3d_filename)
{

}

void x3d_exporter::export_to_sphere(){
    envelop_header_and_footer([this]{
        this->export_all_points();
    });
}

static std::string get_sphere(const std::string reference_name){
    return fmt::scat("<Shape DEF=\"",reference_name, "\">" "\n"
               "  <Sphere radius='1.0' /> " "\n"
               "  <Appearance>" "\n"
               "    <Material DEF='MaterialLightBlue' diffuseColor='0.1 0.5 1'/>" "\n"
               "  </Appearance>" "\n"
               "</Shape>        " "\n");
}


void x3d_exporter::export_all_points(){
    fmt::scat(x3d_stream, "    <Group>" "\n");

    const std::string sphere_unit_name = "baseSphere";

    // export base sphere
    fmt::scat(x3d_stream, get_sphere(sphere_unit_name));


    morpho_tree tree = reader.create_morpho_tree(morpho::h5_v1::morpho_reader::generate_single_soma);

    for(std::size_t b_id =0; b_id < tree.get_tree_size(); ++b_id){

        auto & branch = tree.get_branch(b_id);
        auto & points = branch.get_points();
        auto & distance = branch.get_distances();

        assert(points.size1() == distance.size());
        for(std::size_t i = 0; i < points.size1(); ++i){
            points_distance_to_sphere({ points(i,0), points(i,1), points(i,2) }, distance[i],
                                      sphere_unit_name,
                                      x3d_stream);
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


    html_viewer();

}

void x3d_exporter::html_viewer(){
    fs::path html_filename = fs::path(dest_filename).parent_path();
    html_filename /= fs::path(dest_filename).stem();

    std::ofstream ohtml( fmt::scat(html_filename.native(), "_page.html"));

    fmt::scat(ohtml,
              "<html>" "\n"
              "<head>" "\n"
              "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>" "\n"
              "    <title>Rendering test page for ", dest_filename, " </title>" "\n"
              "    <script type='text/javascript' src='http://www.x3dom.org/download/x3dom.js'> </script>" "\n"
              "    <link rel='stylesheet' type='text/css' href='http://www.x3dom.org/download/x3dom.css'/>" "\n"
              "</head>" "\n"
              "<body>" "\n"
              "<h1>Rendering test page for ", dest_filename, "</h1>" "\n"
              "<script>" "\n"
              "</script>" "\n"
              "<x3d width='1200px' height='800px'>" "\n"
              "    <scene>" "\n"
              "            <Inline nameSpaceName=\"Morpho\" mapDEFToID=\"true\"" "\n"
              "                     url=\"", dest_filename, "\" />" "\n"
              "    </scene>" "\n"
              "</x3d>" "\n"
              "</body>" "\n"
              "</html>");
}

} // morpho
