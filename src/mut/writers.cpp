#include <cassert>

#include <morphio/mut/writers.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>

namespace morphio
{
namespace mut
{
namespace writer
{
void swc(const Morphology& morphology)
{
    // int segmentIdOnDisk = 1;
    // std::map<uint32_t, int32_t> newIds;
    // auto soma = morphology.soma();
    // for (int i = 0; i < soma->points().size(); ++i){
    //     std::cout << segmentIdOnDisk++ << ' ' << SECTION_SOMA << ' '
    //               << soma->points()[i][0] << ' ' << soma->points()[i][1] << ' '
    //               << soma->points()[i][2] << ' ' << soma->diameters()[i] / 2. << ' '
    //               << (i==0 ? -1 : segmentIdOnDisk-1) << std::endl;
    // }

    // auto writeSection =
    //     [&segmentIdOnDisk, &newIds](Morphology& morphology, uint32_t sectionId) {
    //     auto section = morphology.section(sectionId);
    //     const auto& points = section->points();
    //     const auto& diameters = section->diameters();

    //     assert(points.size() > 0 && "Empty section");
    //     for (int i = 1; i < points.size(); ++i)
    //     {
    //         std::cout << segmentIdOnDisk << ' ' << section->type() << ' '
    //                   << points[i][0] << ' ' << points[i][1] << ' '
    //                   << points[i][2] << ' ' << diameters[i] / 2. << ' ';
    //         if (i > 0)
    //             std::cout << segmentIdOnDisk - 1 << std::endl;
    //         else {
    //             uint32_t parentId = morphology.parent(sectionId);
    //             std::cout << (parentId == -1 ? newIds[parentId] : 1) << std::endl;
    //         }

    //         ++segmentIdOnDisk;
    //     }
    //     newIds[section->id()] = segmentIdOnDisk - 1;
    // };


    // for(int sectionId = 0; i<sections().size(); ++i)
    //     writeSection
}

void _write_asc_points(const Points& points,
                       const std::vector<float>& diameters, int indentLevel)
{
    for (int i = 0; i < points.size(); ++i)
    {
        std::cout << std::string(indentLevel, ' ') << "(" << points[i][0] << ' '
                  << points[i][1] << ' ' << points[i][2] << ' ' << diameters[i]
                  << ')' << std::endl;
    }
}

void _write_asc_section(const Morphology& morpho, uint32_t id, int indentLevel)
{
    std::string indent(indentLevel, ' ');
    auto section = morpho.section(id);
    _write_asc_points(section->points(), section->diameters(), indentLevel);

    if (!morpho.children(id).empty())
    {
        std::cout << indent << "(" << std::endl;
        auto it = morpho.children(id).begin();
        _write_asc_section(morpho, (*it++), indentLevel + 2);
        for (; it != morpho.children(id).end(); ++it)
        {
            std::cout << indent << "|" << std::endl;
            _write_asc_section(morpho, (*it), indentLevel + 2);
        }
        std::cout << indent << ")" << std::endl;
    }
}

void asc(const Morphology& morphology)
{
    // std::map<morphio::SectionType, std::string> header;
    // header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    // header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";

    // const auto soma = morphology.soma();
    // std::cout << "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
    // _write_asc_points(soma->points(), soma->diameters(), 2);
    // std::cout << ")\n\n";

    // for (auto& id : morphology.rootSections())
    // {
    //     std::cout << header[morphology.section(id)->type()];
    //     _write_asc_section(morphology, id, 2);
    //     std::cout << ")\n\n";
    // }
}

void h5(const Morphology& morphology)
{
    // using std::setw;
    // int i = 0;

    // int sectionIdOnDisk = 1;
    // std::map<uint32_t, int32_t> newIds;
    // Property::Properties properties;
    // auto writeSection = [&properties, &sectionIdOnDisk,
    //                      &newIds](Morphology& morpho, uint32_t sectionId) {
    //     uint32_t parentId = morpho.parent(sectionId);
    //     int parentOnDisk = (parentId != -1 ? newIds[parentId] : 1);
    //     auto section = morpho.section(sectionId);
    //     std::cout << setw(6) << sectionIdOnDisk << ' ' << setw(6)
    //     << parentOnDisk << ' ' << setw(6) << section->points().size() << ' ' << setw(6)
    //               << section->type() << std::endl;
    //     newIds[section->id()] = sectionIdOnDisk++;
    // };

    // // Hard-coding soma line. Not so swag
    // // Will we have morphology without soma ?
    // std::cout << setw(6) << 0 << ' ' << setw(6) << -1 << ' ' << setw(6) << 0
    //           << ' ' << setw(6) << SECTION_SOMA << std::endl;

    // // morphology.traverse(writeSection);
}

} // end namespace writer
} // end namespace mut
} // end namespace morphio
