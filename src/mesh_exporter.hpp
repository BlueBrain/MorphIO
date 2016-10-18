#ifndef MESH_EXPORTER_H
#define MESH_EXPORTER_H

#include <fstream>
#include <iostream>
#include <string>
#include <bitset>
#include <functional>
#include <unordered_set>

#include <hadoken/geometry/geometry.hpp>
#include <hadoken/math/math_floating_point.hpp>

#include <morpho/morpho_h5_v1.hpp>
#include <morpho/morpho_tree.hpp>


namespace morpho{

namespace geo = hadoken::geometry::cartesian;

///
/// \brief gmsh representation of a point
///
struct gmsh_point{
    inline gmsh_point(const geo::point3d & my_coords, double d = 0) :
        coords(my_coords),
        diameter(d),
        id(0),
        isPhysical(false){}

    inline void setPhysical(bool phys){
        isPhysical = phys;
    }

    inline geo::point3d get_point() const{
        return coords;
    }


    inline bool operator==(const gmsh_point & other) const noexcept{
        return ( hadoken::math::almost_equal(geo::get_x(coords), geo::get_x(other.coords))
               && hadoken::math::almost_equal(geo::get_y(coords), geo::get_y(other.coords))
               && hadoken::math::almost_equal(geo::get_z(coords), geo::get_z(other.coords)));
    }

    geo::point3d coords;
    double diameter;
    std::size_t id;
    bool isPhysical;
};


///
struct gmsh_segment{
    inline gmsh_segment(const gmsh_point & p1, const gmsh_point & p2) :
        point1(p1), point2(p2),
        id(0),
        isPhysical(false){}

    inline void setPhysical(bool phys){
        isPhysical = phys;
    }

    /// operator < for Comparable requirements
    inline bool operator==(const gmsh_segment & other) const noexcept{
        return (point1 == other.point1) && (point2 == other.point2);
    }


    gmsh_point point1, point2;
    std::size_t id;
    bool isPhysical;
};

///
/// \brief in memory abstract gmsh file representation
///
class gmsh_abstract_file : private boost::noncopyable{
public:

    /// add a point and return its id
    std::size_t add_point(const gmsh_point & point);

    std::size_t find_point(const gmsh_point & point);

    /// add a segment and return its id
    std::size_t add_segment(const gmsh_segment & s);

    /// get all points
    std::vector<gmsh_point> get_all_points() const;

    /// get all segments
    std::vector<gmsh_segment> get_all_segments() const;

    /// export all points to stream in gmsh format
    void export_points_to_stream(std::ostream & out);

    /// export all segments to stream in gmsh format
    void export_segments_to_stream(std::ostream & out);

private:
    struct hash_gmsh_point{
        std::size_t operator()(const gmsh_point & p) const noexcept{
            std::hash<double> hd;
            return hd(geo::get_x(p.get_point()));
        }
    };

    struct hash_gmsh_segment{
        std::size_t operator()(const gmsh_segment & s) const noexcept{
            std::hash<double> hd;
            return hd(geo::get_x(s.point1.coords));
        }
    };

    std::unordered_set<gmsh_point, hash_gmsh_point> _points;

    std::unordered_set<gmsh_segment, hash_gmsh_segment> _segments;
};


///
/// \brief The gmsh_exporter class
///
class gmsh_exporter
{
public:

    typedef std::bitset<64> exporter_flags;
    static constexpr int exporter_single_soma = 0x01;


    gmsh_exporter(const std::string & morphology_filename, const std::string & mesh_filename, exporter_flags flags = exporter_flags());


    void export_to_point_cloud();

    void export_to_wireframe();



private:
    std::ofstream geo_stream;
    morpho::h5_v1::morpho_reader reader;
    exporter_flags flags;


    void serialize_header();

    void serialize_points_raw();


   void construct_gmsh_vfile_raw(gmsh_abstract_file & vfile);

   void construct_gmsh_vfile_lines(morpho_tree & tree, branch & current_branch, gmsh_abstract_file & vfile);

};


} //morpho

#endif // MESH_EXPORTER_H
