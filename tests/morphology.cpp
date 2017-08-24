/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <brion/brion.h>
#include <tests/paths.h>

#ifdef BRION_USE_ZEROEQ
#include <brion/constants.h>
#include <zeroeq/server.h>
#include <zeroeq/uri.h>
#endif

#define BOOST_TEST_MODULE Morphology
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#include <cstdarg>

// typedef for brevity
typedef brion::Vector4f V4f;
typedef brion::Vector2i V2i;

namespace
{
// Ellipsis promotes enums to ints, so we need to use int.
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
const int UNDEFINED = brion::SECTION_UNDEFINED;
const int SOMA = brion::SECTION_SOMA;
const int AXON = brion::SECTION_AXON;
const int DENDRITE = brion::SECTION_DENDRITE;
const int APICAL_DENDRITE = brion::SECTION_APICAL_DENDRITE;
}

template <typename T>
void checkCloseArrays(const std::vector<T>& array1,
                      const std::vector<T>& array2)
{
    BOOST_CHECK_EQUAL(array1.size(), array2.size());
    for (size_t i = 0; i != std::min(array1.size(), array2.size()); ++i)
        BOOST_CHECK_CLOSE(array1[i], array2[i], 2e-5f);
}

BOOST_AUTO_TEST_CASE(invalid_open)
{
    BOOST_CHECK_THROW(brion::Morphology(brion::URI("/bla")).getPoints(),
                      std::runtime_error);
    BOOST_CHECK_THROW(brion::Morphology(brion::URI("bla")).getPoints(),
                      std::runtime_error);

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/README";
    BOOST_CHECK_THROW(brion::Morphology{brion::URI(path.string())}.getPoints(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(h5_invalid_open)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/simulations/may17_2011/Control/voltage.h5";
    BOOST_CHECK_THROW(brion::Morphology{brion::URI(path.string())}.getPoints(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(h5_read_v1)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/morphologies/01.07.08/h5/R-C010306G.h5";

    const brion::Morphology morphology{brion::URI(path.string())};
    BOOST_CHECK_EQUAL(morphology.getCellFamily(), brion::FAMILY_NEURON);

    const auto points = morphology.getPoints();
    BOOST_CHECK_EQUAL(points.size(), 3272);
    BOOST_CHECK_CLOSE(points[0].x(), -9.0625f, .000001f);
    BOOST_CHECK_CLOSE(points[0].y(), -4.97781f, .0001f);
    BOOST_CHECK_CLOSE(points[0].z(), 0.f, .000001f);
    BOOST_CHECK_CLOSE(points[0].w(), 0.37f, .000001f);

    const auto sections = morphology.getSections();
    BOOST_CHECK_EQUAL(sections.size(), 138);
    BOOST_CHECK_EQUAL(sections[0].x(), 0);
    BOOST_CHECK_EQUAL(sections[0].y(), -1);
    BOOST_CHECK_EQUAL(sections[5].x(), 85);
    BOOST_CHECK_EQUAL(sections[5].y(), 4);

    const auto types = morphology.getSectionTypes();
    BOOST_CHECK_EQUAL(types.size(), 138);
    BOOST_CHECK_EQUAL(types[0], 1);
    BOOST_CHECK_EQUAL(types[5], 2);

    BOOST_CHECK(morphology.getPerimeters().empty());
}

namespace
{
void _checkH5V2(const brion::Morphology& morphology)
{
    BOOST_CHECK_EQUAL(morphology.getCellFamily(), brion::FAMILY_NEURON);

    auto& points = morphology.getPoints();
    BOOST_CHECK_EQUAL(points.size(), 1499);
    BOOST_CHECK_CLOSE(points[0].x(), 5.335999965667725f, .000001f);
    BOOST_CHECK_CLOSE(points[0].y(), 2.702667474746704f, .000001f);
    BOOST_CHECK_CLOSE(points[0].z(), -1.1733332872390747f, .000001f);
    BOOST_CHECK_CLOSE(points[0].w(), 0.f, .000001f);

    auto& sections = morphology.getSections();
    BOOST_CHECK_EQUAL(sections.size(), 78);
    BOOST_CHECK_EQUAL(sections[0].x(), 0);
    BOOST_CHECK_EQUAL(sections[0].y(), -1);
    BOOST_CHECK_EQUAL(sections[5].x(), 49);
    BOOST_CHECK_EQUAL(sections[5].y(), 4);

    auto& types = morphology.getSectionTypes();
    BOOST_CHECK_EQUAL(types.size(), 78);
    BOOST_CHECK_EQUAL(types[0], 1);
    BOOST_CHECK_EQUAL(types[5], 2);

    BOOST_CHECK(morphology.getPerimeters().empty());
}
}

BOOST_AUTO_TEST_CASE(h5_read_v2)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/morphologies/14.07.10_repaired/v2/C010398B-P2.h5";

    const brion::Morphology morphology{brion::URI(path.string())};
    _checkH5V2(morphology);
}

BOOST_AUTO_TEST_CASE(copy_morphology)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/morphologies/14.07.10_repaired/v2/C010398B-P2.h5";

    const brion::Morphology morphology{brion::URI(path.string())};
    const brion::Morphology copy(morphology);

    boost::filesystem::path v1(BBP_TESTDATA);
    v1 /= "local/morphologies/01.07.08/h5/R-C010306G.h5";
    brion::Morphology assign{brion::URI(v1.string())};
    assign = copy;

    _checkH5V2(morphology);
    _checkH5V2(copy);
    _checkH5V2(assign);
}

BOOST_AUTO_TEST_CASE(move_morphology)
{
    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/morphologies/14.07.10_repaired/v2/C010398B-P2.h5";

    brion::Morphology morphology{brion::URI(path.string())};
    const brion::Morphology move(std::move(morphology));
    _checkH5V2(move);

    morphology = std::move(move);
    _checkH5V2(morphology);
}

#ifdef BRION_USE_ZEROEQ
BOOST_AUTO_TEST_CASE(zeroeq_read)
{
    zeroeq::Server server(zeroeq::NULL_SESSION);
    server.handle(brion::ZEROEQ_GET_MORPHOLOGY,
                  [](const void* data, const size_t size) {
                      if (!data || !size)
                          return zeroeq::ReplyData();

                      const std::string path((const char*)data, size);
                      const brion::Morphology morphology{brion::URI(path)};
                      return zeroeq::ReplyData(brion::ZEROEQ_GET_MORPHOLOGY,
                                               morphology.toBinary().clone());
                  });
    std::thread thread([&] { server.receive(); });

    boost::filesystem::path path(BBP_TESTDATA);
    path /= "local/morphologies/14.07.10_repaired/v2/C010398B-P2.h5";

    const brion::URI uri{std::string("zeroeq://") + server.getURI().getHost() +
                         ":" + std::to_string(int(server.getURI().getPort())) +
                         path.string()};

    brion::Morphology morphology(uri);
    thread.join();

    _checkH5V2(morphology);
}
#endif

BOOST_AUTO_TEST_CASE(swc_invalid_open)
{
    BOOST_CHECK_THROW(
        brion::Morphology(brion::URI("not_found.swc")).getPoints(),
        std::runtime_error);
    try
    {
        boost::filesystem::path path(BRION_TESTDATA);
        path /= "swc/bad_syntax.swc";
        brion::Morphology(brion::URI(path.string())).getPoints();
        BOOST_CHECK(false);
    }
    catch (std::runtime_error& error)
    {
        BOOST_CHECK(std::string(error.what()).find("line 6") !=
                    std::string::npos);
    }
}

template <typename T>
std::ostream& operator<<(std::ostream& out, std::vector<T>& list)
{
    for (T i : list)
        out << i << ' ';
    out << std::endl;
    return out;
}

std::ostream& operator<<(std::ostream& out,
                         std::vector<brion::SectionType>& list)
{
    for (brion::SectionType i : list)
        out << (int)i << ' ';
    out << std::endl;
    return out;
}

template <typename T>
struct VaArgsType
{
    typedef T type;
};

template <>
struct VaArgsType<brion::SectionType>
{
    typedef int type;
};

template <typename T>
void checkEqualArrays(const std::vector<T>& array, const size_t length, ...)
{
    // Create the reference array
    std::vector<T> ref;
    va_list args;
    va_start(args, length);
    for (size_t i = 0; i != length; ++i)
        ref.push_back((T)va_arg(args, typename VaArgsType<T>::type));
    va_end(args);

    BOOST_CHECK_EQUAL_COLLECTIONS(array.begin(), array.end(), ref.begin(),
                                  ref.end());
}

template <typename T>
void _checkCloseArrays(const std::vector<T>& array, const size_t length,
                       va_list args)
{
    for (size_t i = 0; i != length; ++i)
    {
        const T& v = (T)va_arg(args, typename VaArgsType<T>::type);
        std::ostringstream os;
        os << array[i] << " != " << v << " at " << i;
        BOOST_CHECK_MESSAGE(array[i].equals(v), os.str());
    }
}

template <typename T>
void checkCloseArrays(const std::vector<T>& array, const size_t length, ...)
{
    BOOST_CHECK_EQUAL(array.size(), length);
    va_list args;
    va_start(args, length);
    _checkCloseArrays(array, length, args);
    va_end(args);
}

template <typename T, long unsigned int M>
void checkCloseArrays(const std::vector<vmml::vector<M, T>>& array1,
                      const std::vector<vmml::vector<M, T>>& array2)
{
    BOOST_CHECK_EQUAL(array1.size(), array2.size());
    for (size_t i = 0; i != std::min(array1.size(), array2.size()); ++i)
        BOOST_CHECK_SMALL((array1[i] - array2[i]).length(), 0.00001f);
}

template <typename T>
void checkCloseArraysUptoN(const std::vector<T>& array, const size_t length,
                           ...)
{
    BOOST_CHECK(array.size() >= length);
    va_list args;
    va_start(args, length);
    _checkCloseArrays(array, length, args);
    va_end(args);
}

BOOST_AUTO_TEST_CASE(swc_soma)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/soma.swc";

    const brion::Morphology source{brion::URI(path.string())};
    checkEqualArrays(source.getPoints(), 1, V4f(0, 0, 0, 20));
    checkEqualArrays(source.getSections(), 1, V2i(0, -1));
    checkEqualArrays(source.getSectionTypes(), 1, SOMA);
}

BOOST_AUTO_TEST_CASE(swc_soma_ring)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/soma_ring.swc";

    const brion::Morphology source{brion::URI(path.string())};
    checkEqualArrays(source.getPoints(), 5, V4f(0, 0, 0, 20), V4f(0, 0, 1, 20),
                     V4f(0, 1, 0, 20), V4f(0, 1, 1, 20), V4f(1, 0, 0, 20));
    checkEqualArrays(source.getSections(), 1, V2i(0, -1));
    checkEqualArrays(source.getSectionTypes(), 1, SOMA);
}

BOOST_AUTO_TEST_CASE(swc_no_soma)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/no_soma.swc";

    BOOST_CHECK_THROW(brion::Morphology{brion::URI(path.string())}.getPoints(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(swc_two_somas)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/two_somas.swc";

    BOOST_CHECK_THROW(brion::Morphology{brion::URI(path.string())}.getPoints(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(swc_single_section)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/single_section.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getPoints(), 5, V4f(0, 0, 0, 20), V4f(0, 0, 1, 4),
                     V4f(0, 0, 2, 4), V4f(0, 0, 3, 4), V4f(0, 0, 4, 4));
    checkEqualArrays(source.getSections(), 2, V2i(0, -1), V2i(1, 0));
    checkEqualArrays(source.getSectionTypes(), 2, SOMA, AXON);
}

BOOST_AUTO_TEST_CASE(swc_single_section_unordered)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/single_section_unordered.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getPoints(), 5, V4f(0, 0, 0, 20), V4f(0, 0, 1, 4),
                     V4f(0, 0, 2, 4), V4f(0, 0, 3, 4), V4f(0, 0, 4, 4));
    checkEqualArrays(source.getSections(), 2, V2i(0, -1), V2i(1, 0));
    checkEqualArrays(source.getSectionTypes(), 2, SOMA, AXON);
}

BOOST_AUTO_TEST_CASE(swc_single_section_missing_segment)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/single_section_missing_segment.swc";

    BOOST_CHECK_THROW(brion::Morphology{brion::URI(path.string())}.getPoints(),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(swc_section_type_changes)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/section_type_changes.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getPoints(), 7, V4f(0, 0, 0, 20), V4f(0, 0, 1, 4),
                     V4f(0, 0, 2, 4), V4f(0, 0, 2, 4), V4f(0, 0, 3, 4),
                     V4f(0, 0, 3, 4), V4f(0, 0, 4, 4));
    checkEqualArrays(source.getSections(), 4, V2i(0, -1), V2i(1, 0), V2i(3, 1),
                     V2i(5, 2));
    checkEqualArrays(source.getSectionTypes(), 4, SOMA, AXON, DENDRITE,
                     APICAL_DENDRITE);
}

BOOST_AUTO_TEST_CASE(swc_first_order_sections)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/first_order_sections.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getSections(), 4, V2i(0, -1), V2i(1, 0), V2i(2, 0),
                     V2i(3, 0));
    // The tree construction algorithm reverses the order of the sections
    // compared to how they appear in the file
    checkEqualArrays(source.getPoints(), 4, V4f(0, 0, 0, 20), V4f(0, 0, 3, 4),
                     V4f(0, 0, 2, 4), V4f(0, 0, 1, 4));
    checkEqualArrays(source.getSectionTypes(), 4, SOMA, APICAL_DENDRITE,
                     DENDRITE, AXON);
}

BOOST_AUTO_TEST_CASE(swc_first_order_sections_from_arbitrary_points)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/first_order_sections_ring.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getSections(), 4, V2i(0, -1), V2i(5, 0), V2i(8, 0),
                     V2i(11, 0));
    // The tree construction algorithm reverses the order of the sections
    // compared to how they appear in the file
    checkEqualArrays(source.getPoints(), 14, V4f(0, 0, 1, 0), V4f(0, 0, 2, 0),
                     V4f(0, 0, 3, 0), V4f(0, 0, 4, 0), V4f(0, 0, 5, 0),
                     V4f(0, 0, 4, 0), V4f(3, 1, 10, 1), V4f(3, 2, 11, 1),
                     V4f(0, 0, 3, 0), V4f(2, 1, 8, 1), V4f(2, 2, 9, 1),
                     V4f(0, 0, 2, 0), V4f(1, 1, 6, 1), V4f(1, 2, 7, 1));
    checkEqualArrays(source.getSectionTypes(), 4, SOMA, APICAL_DENDRITE,
                     DENDRITE, AXON);
}

BOOST_AUTO_TEST_CASE(swc_bifurcation)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/bifurcations.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getPoints(), 9, V4f(0, 0, 0, 20), V4f(0, 0, 2, 4),
                     V4f(0, 0, 3, 4), V4f(0, 0, 3, 4), V4f(0, 0, 4, 4),
                     V4f(0, 0, 5, 4), V4f(0, 0, 3, 4), V4f(0, 0, 6, 4),
                     V4f(0, 0, 7, 4));
    checkEqualArrays(source.getSections(), 4, V2i(0, -1), V2i(1, 0), V2i(3, 1),
                     V2i(6, 1));
    checkEqualArrays(source.getSectionTypes(), 4, SOMA, DENDRITE,
                     APICAL_DENDRITE, APICAL_DENDRITE);
}

BOOST_AUTO_TEST_CASE(swc_end_points)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/end_points.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getSections(), 6, V2i(0, -1), V2i(1, 0), V2i(2, 0),
                     V2i(3, 0), V2i(4, 3), V2i(6, 3));

    checkEqualArrays(source.getSectionTypes(), 6, SOMA, UNDEFINED, UNDEFINED,
                     AXON, AXON, AXON);
}

BOOST_AUTO_TEST_CASE(swc_fork_points)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/fork_points.swc";

    const brion::Morphology source{brion::URI(path.string())};

    checkEqualArrays(source.getSections(), 6, V2i(0, -1), V2i(1, 0), V2i(2, 0),
                     V2i(3, 0), V2i(4, 3), V2i(6, 3));

    checkEqualArrays(source.getSectionTypes(), 6, SOMA, UNDEFINED, UNDEFINED,
                     AXON, AXON, AXON);
}

BOOST_AUTO_TEST_CASE(swc_neuron)
{
    boost::filesystem::path path(BRION_TESTDATA);
    path /= "swc/Neuron.swc";

    const brion::Morphology neuron{brion::URI(path.string())};
    BOOST_CHECK_EQUAL(neuron.getPoints().size(), 933);
    BOOST_CHECK_EQUAL(neuron.getCellFamily(), brion::FAMILY_NEURON);
    BOOST_CHECK(neuron.getPerimeters().empty());
}
