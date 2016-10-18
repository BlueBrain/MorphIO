#define BOOST_TEST_MODULE testGmsh
#define BOOST_TEST_MAIN

#include <iostream>
#include <sstream>
#include <memory>

#include <boost/test/unit_test.hpp>

#include <morpho/morpho_tree.hpp>
#include <src/mesh_exporter.hpp>


BOOST_AUTO_TEST_CASE( test_gmsh_point )
{
    using namespace morpho;
    namespace geo = hadoken::geometry::cartesian;

    gmsh_abstract_file mfile;

    gmsh_point p1(geo::point3d(0.5, 0.2, 0.6)), p2(geo::point3d(0.5, 0.2, 0.7)), p3(geo::point3d(1, 2, 3));

    gmsh_point p1_copy(p1);

    BOOST_CHECK( (p1 == p1_copy ) );
    BOOST_CHECK( (p1 == p2 ) == false);


    BOOST_CHECK_EQUAL(mfile.add_point(p1), 0);
    BOOST_CHECK_EQUAL(mfile.add_point(p2), 1);

    // check if duplicate insert filtering is working
    BOOST_CHECK_EQUAL(mfile.add_point(p1), 0);
    BOOST_CHECK_EQUAL(mfile.add_point(p2), 1);

    BOOST_CHECK_EQUAL(mfile.add_point(p3), 2);

    auto all_points = mfile.get_all_points();

    BOOST_CHECK_EQUAL(all_points.size(), 3);
}




BOOST_AUTO_TEST_CASE( test_gmsh_lines )
{
    using namespace morpho;
    namespace geo = hadoken::geometry::cartesian;

    gmsh_abstract_file mfile;

    gmsh_point p1(geo::point3d(0.5, 0.2, 0.6)), p2(geo::point3d(0.5, 0.2, 0.7)), p3(geo::point3d(1, 2, 3));

    gmsh_segment segment1(p1, p2), segment2(p2, p3);

    std::size_t id1 = mfile.add_segment(segment1);
    std::size_t id2 = mfile.add_segment(segment2);

    BOOST_CHECK_EQUAL(id1, 0);
    BOOST_CHECK_EQUAL(id2, 1);

    BOOST_CHECK_EQUAL(mfile.get_all_points().size(), 3);

    gmsh_point p11(geo::point3d(0.5, 0.2, 0.6)), p12(geo::point3d(0.5, 0.2, 0.7));
    std::size_t id3 = mfile.add_segment(gmsh_segment(p11, p12));

    BOOST_CHECK_EQUAL(id1, id3);

    BOOST_CHECK_EQUAL(mfile.get_all_segments().size(), 2);

    BOOST_CHECK_EQUAL(mfile.find_point(geo::point3d(0.5, 0.2, 0.6)), 0);

}


