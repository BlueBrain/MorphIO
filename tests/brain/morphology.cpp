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

#include <brain/brain.h>
#include <brion/brion.h>
#include <tests/paths.h>

#define BOOST_TEST_MODULE MorphologyBrain
#include <boost/test/unit_test.hpp>
#include <cstdarg>

// typedef for brevity
typedef brion::Vector4f V4f;
typedef brion::Vector3f V3f;

namespace
{
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
void checkCloseArrays(const std::vector<T>& array1,
                      const std::vector<T>& array2)
{
    BOOST_CHECK_EQUAL(array1.size(), array2.size());
    for (size_t i = 0; i != std::min(array1.size(), array2.size()); ++i)
        BOOST_CHECK_CLOSE(array1[i], array2[i], 2e-5f);
}

template <typename T>
void checkEqualArrays(const std::vector<T>& array1,
                      const std::vector<T>& array2)
{
    BOOST_CHECK_EQUAL_COLLECTIONS(array1.begin(), array1.end(), array2.begin(),
                                  array2.end());
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
void checkCloseArraysUpToGiven(const std::vector<T>& array,
                               const std::vector<T>& given)
{
    BOOST_CHECK_GE(array.size(), given.size());
    checkCloseArrays(std::vector<T>(array.begin(),
                                    array.begin() + given.size()),
                     given);
}

brion::uint32_ts getSectionIDs(const brain::neuron::Sections& sections)
{
    brion::uint32_ts result;
    for (const brain::neuron::Section& section : sections)
        result.push_back(section.getID());
    return result;
}

const std::string TEST_MORPHOLOGY_FILENAME =
    std::string(BRION_TESTDATA) + "/h5/test_neuron.h5";
const brion::URI TEST_MORPHOLOGY_URI =
    brion::URI("file://" + TEST_MORPHOLOGY_FILENAME);

void checkEqualMorphologies(const brain::neuron::Morphology& first,
                            const brion::Morphology& second)
{
    BOOST_CHECK(*second.readPoints() == first.getPoints());
    BOOST_CHECK(*second.readSections() == first.getSections());
    BOOST_CHECK(
        *second.readSectionTypes() ==
        reinterpret_cast<const brion::SectionTypes&>(first.getSectionTypes()));
    BOOST_CHECK(*second.readApicals() == first.getApicals());
}
} // namespace

BOOST_AUTO_TEST_CASE(v2_morphology_constructors)
{
    boost::shared_ptr<brion::Morphology> raw(
        new brion::Morphology(TEST_MORPHOLOGY_FILENAME));

    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);
    BOOST_CHECK_EQUAL(morphology.getTransformation(), brain::Matrix4f());
    checkEqualMorphologies(morphology, *raw);
    checkEqualMorphologies(brain::neuron::Morphology(*raw), *raw);

    BOOST_CHECK_THROW(brain::neuron::Morphology(brion::URI("/mars")),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(get_section_ids)
{
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);
    using brain::neuron::SectionType;

    brain::neuron::SectionTypes types{SectionType::soma};
    checkEqualArrays(morphology.getSectionIDs(types), {0});

    types.push_back(SectionType::dendrite);
    checkEqualArrays(morphology.getSectionIDs(types), {0, 4, 5, 6, 7, 8, 9});
    types.push_back(SectionType::apicalDendrite);
    checkEqualArrays(morphology.getSectionIDs(types),
                     {0, 4, 5, 6, 7, 8, 9, 10, 11, 12});
    types.clear();
    types.push_back(SectionType::axon);
    types.push_back(SectionType::dendrite);
    checkEqualArrays(morphology.getSectionIDs(types),
                     {1, 2, 3, 4, 5, 6, 7, 8, 9});
}

BOOST_AUTO_TEST_CASE(get_sections)
{
    using brain::neuron::SectionType;
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);

    BOOST_CHECK_THROW(morphology.getSection(0), std::runtime_error);

    for (size_t i = 1; i < 13; ++i)
        BOOST_CHECK_EQUAL(morphology.getSection(i).getID(), i);

    brain::neuron::Section section = morphology.getSection(1);
    BOOST_CHECK(section == morphology.getSection(1));
    section = morphology.getSection(2);
    BOOST_CHECK(section != morphology.getSection(1));
    BOOST_CHECK(section == morphology.getSection(2));

    for (size_t i = 1; i < 4; ++i)
        BOOST_CHECK(morphology.getSection(i).getType() == SectionType::axon);
    for (size_t i = 4; i < 10; ++i)
        BOOST_CHECK(morphology.getSection(i).getType() ==
                    SectionType::dendrite);
    for (size_t i = 10; i < 13; ++i)
        BOOST_CHECK(morphology.getSection(i).getType() ==
                    SectionType::apicalDendrite);
}

BOOST_AUTO_TEST_CASE(get_section_samples)
{
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);

    brion::Vector4fs points;
    for (size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(0, -i2 / 20.0, i2 / 20.0, 0.5 + i2 / 1000.0));
    }
    checkCloseArrays(morphology.getSection(1).getSamples(), points);

    points.clear();
    for (size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(i2 / 20.0, 0, i2 / 20.0, 0.5 + i2 / 1000.0));
    }
    checkCloseArrays(morphology.getSection(4).getSamples(), points);

    points.clear();
    for (size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(-i2 / 20.0, 0, i2 / 20.0, 0.5 + i2 / 1000.0));
    }
    checkCloseArrays(morphology.getSection(7).getSamples(), points);

    points.clear();
    for (size_t i = 0; i != 11; ++i)
    {
        float i2 = i * i;
        points.push_back(
            brion::Vector4f(0, i2 / 20.0, i2 / 20.0, 0.5 + i2 / 1000.0));
    }
    checkCloseArrays(morphology.getSection(10).getSamples(), points);
}

BOOST_AUTO_TEST_CASE(get_section_distances_to_soma)
{
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);

    uint32_t sections[] = {1, 4, 7, 10};

    for (size_t i = 0; i != 4; ++i)
    {
        uint32_t section = sections[i];
        BOOST_CHECK_EQUAL(morphology.getSection(section).getDistanceToSoma(),
                          0);
        const float length = std::sqrt(5 * 5 * 2);
        BOOST_CHECK_CLOSE(morphology.getSection(section).getLength(), length,
                          1e-5);

        // The distance to the soma of the next section is equal to the length
        // of its parent
        BOOST_CHECK_CLOSE(
            morphology.getSection(section + 1).getDistanceToSoma(), length,
            1e-5);

        brion::floats reference;
        for (size_t j = 0; j != 11; ++j)
        {
            const float p = j * j / 20.0;
            reference.push_back(std::sqrt(p * p * 2));
        }
        checkCloseArrays(
            morphology.getSection(section).getSampleDistancesToSoma(),
            reference);
    }
}

BOOST_AUTO_TEST_CASE(get_soma_geometry)
{
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);

    const brain::neuron::Soma soma = morphology.getSoma();
    checkEqualArrays(soma.getProfilePoints(),
                     {V4f(.1, 0, 0, .1), V4f(0, .1, 0, .1), V4f(-.1, 0, 0, .1),
                      V4f(0, -.1, 0, .1)});

    BOOST_CHECK_CLOSE(soma.getMeanRadius(), 0.1, 1e-5);
    BOOST_CHECK_EQUAL(soma.getCentroid(), V3f());

    brain::Matrix4f matrix;
    matrix.setTranslation(V3f(2, 0, 0));
    brain::neuron::Morphology transformed(TEST_MORPHOLOGY_URI, matrix);
    BOOST_CHECK_MESSAGE(transformed.getSoma().getCentroid().equals(
                            V3f(2, 0, 0)),
                        transformed.getSoma().getCentroid());
}

BOOST_AUTO_TEST_CASE(get_section_samples_by_positions)
{
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);

    brion::floats points;
    for (float p = 0.0; p <= 1.0; p += 0.2)
        points.push_back(p);

    checkCloseArrays(morphology.getSection(1).getSamples(points),
                     {V4f(0, 0, 0, .5), V4f(0, -1, 1, .52), V4f(0, -2, 2, .54),
                      V4f(0, -3, 3, .56), V4f(0, -4, 4, .58),
                      V4f(0, -5, 5, .6)});

    checkCloseArrays(morphology.getSection(4).getSamples(points),
                     {V4f(0, 0, 0, .5), V4f(1, 0, 1, .52), V4f(2, 0, 2, .54),
                      V4f(3, 0, 3, .56), V4f(4, 0, 4, .58), V4f(5, 0, 5, .6)});

    checkCloseArrays(morphology.getSection(7).getSamples(points),
                     {V4f(0, 0, 0, .5), V4f(-1, 0, 1, .52), V4f(-2, 0, 2, .54),
                      V4f(-3, 0, 3, .56), V4f(-4, 0, 4, .58),
                      V4f(-5, 0, 5, .6)});

    checkCloseArrays(morphology.getSection(10).getSamples(points),
                     {V4f(0, 0, 0, .5), V4f(0, 1, 1, .52), V4f(0, 2, 2, .54),
                      V4f(0, 3, 3, .56), V4f(0, 4, 4, .58), V4f(0, 5, 5, .6)});
}

BOOST_AUTO_TEST_CASE(morphology_hierarchy)
{
    brain::neuron::Morphology morphology(TEST_MORPHOLOGY_URI);

    BOOST_CHECK(!morphology.getSection(1).hasParent());
    BOOST_CHECK(!morphology.getSection(4).hasParent());
    BOOST_CHECK_EQUAL(morphology.getSection(2).getParent().getID(), 1);
    BOOST_CHECK_EQUAL(morphology.getSection(3).getParent().getID(), 1);
    BOOST_CHECK_EQUAL(morphology.getSection(5).getParent().getID(), 4);
    BOOST_CHECK_EQUAL(morphology.getSection(6).getParent().getID(), 4);

    checkEqualArrays(getSectionIDs(morphology.getSoma().getChildren()),
                     {1, 4, 7, 10});
    checkEqualArrays(getSectionIDs(morphology.getSection(1).getChildren()),
                     {2, 3});
    checkEqualArrays(getSectionIDs(morphology.getSection(4).getChildren()),
                     {5, 6});
    BOOST_CHECK(morphology.getSection(5).getChildren().empty());
}

BOOST_AUTO_TEST_CASE(transform_with_matrix)
{
    brain::Matrix4f matrix;
    matrix.rotate_z(M_PI * 0.5);
    brain::neuron::Morphology rotated(TEST_MORPHOLOGY_URI, matrix);
    checkCloseArraysUpToGiven(rotated.getPoints(),
                              {V4f(.0, .1, .0, .1), V4f(-.1, .0, .0, .1),
                               V4f(.0, -.1, .0, .1), V4f(.1, .0, .0, .1)});

    matrix = brain::Matrix4f();
    matrix.rotate_z(M_PI * 0.5);
    matrix.setTranslation(V3f(2, 0, 0));
    brain::neuron::Morphology transformed(TEST_MORPHOLOGY_URI, matrix);
    BOOST_CHECK_EQUAL(transformed.getTransformation(), matrix);
    checkCloseArraysUpToGiven(transformed.getPoints(),
                              {V4f(2., .1, .0, .1), V4f(1.9, .0, .0, .1),
                               V4f(2., -.1, .0, .1), V4f(2.1, .0, .0, .1)});
}
