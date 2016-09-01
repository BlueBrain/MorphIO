/* Copyright (c) 2013-2016, EPFL/Blue Brain Project
 *                          Juan Hernando <juan.hernando@epfl.ch>
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

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>

#include "../arrayHelpers.h"
#include "../helpers.h"

#include <brain/neuron/morphology.h>
#include <brain/neuron/section.h>
#include <brain/neuron/soma.h>

#include <vmmlib/matrix.hpp>
#include <vmmlib/vector.hpp>

namespace bp = boost::python;

namespace brain
{
namespace neuron
{

namespace
{

template< typename Part >
class MorphologyPartWrapper : public Part
{
public:
    MorphologyPartWrapper( const Part& part, const MorphologyPtr& morphology_ )
        : Part( part )
        , morphology( morphology_ )
    {}

    MorphologyPtr morphology;
};

typedef MorphologyPartWrapper<Soma> SomaWrapper;
typedef MorphologyPartWrapper<Section> SectionWrapper;

bp::object Soma_getProfilePoints( const SomaWrapper& soma )
{
    return toNumpy( soma.getProfilePoints( ));
}

#define GET_SECTION_ARRAY( Array ) \
    bp::object Section_get##Array( const SectionWrapper& section )  \
    {                                                               \
        return toNumpy( section.get##Array( ));                     \
    }

GET_SECTION_ARRAY( Samples )
GET_SECTION_ARRAY( SampleDistancesToSoma )

bp::object Section_getSamplesFromPositions( const SectionWrapper& section,
                                            bp::object points )
{
    const floats pointVector = vectorFromPython< float >(
        points, "Cannot convert argument to float vector");
    return toNumpy( section.getSamples( pointVector ));
}

bp::object Section_getParent( const SectionWrapper& section )
{
    if( section.hasParent( ))
        return bp::object( SectionWrapper( section.getParent( ),
                                           section.morphology ));
    return bp::object();
}

bp::object Section_getChildren( const SectionWrapper& section )
{
    const Sections& sections = section.getChildren();
    bp::list result;
    for( const auto& i : sections )
        result.append( SectionWrapper( i, section.morphology ));
    return result;
}

MorphologyPtr Morphology_initFromURI( const std::string& uri )
{
    return MorphologyPtr( new Morphology( URI( uri )));
}

MorphologyPtr Morphology_initFromURIAndTransform( const std::string& uri,
                                                  bp::object transform )
{
    return MorphologyPtr( new Morphology( URI( uri ), fromNumpy( transform )));
}

#define GET_MORPHOLOGY_ARRAY( Array ) \
    bp::object Morphology_get##Array( const MorphologyPtr& morphology ) \
    {                                                                   \
        return toNumpy( morphology->get##Array(), morphology );         \
    }

GET_MORPHOLOGY_ARRAY( Points )
GET_MORPHOLOGY_ARRAY( Sections )
GET_MORPHOLOGY_ARRAY( Apicals )

bp::object Morphology_getSectionIDs( const MorphologyPtr& morphology,
                                     bp::object types )
{
    const SectionTypes typeVector = vectorFromPython< SectionType >(
        types, "Cannot convert argument to SectionType list");
    return toNumpy( morphology->getSectionIDs( typeVector ));
}

bp::object Morphology_getSectionTypes( const MorphologyPtr& morphology )
{
    return toNumpy( morphology->getSectionTypes( ), morphology );
}

bp::object Morphology_getSectionsByType( const MorphologyPtr& morphology,
                                         bp::object types )
{
    SectionTypes typeVector;
    try
    {
        bp::extract< SectionType > extractor( types );
        if( extractor.check( ))
        {
            typeVector.push_back(( SectionType )extractor );
        }
        else
        {
            bp::stl_input_iterator< SectionType > i( types ), end;
            for( ; i != end; ++i )
                typeVector.push_back( *i );
        }
    }
    catch(...)
    {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot convert argument to SectionType list");
        bp::throw_error_already_set();
    }
    const Sections& sections = morphology->getSections( typeVector );
    bp::list result;
    for( const auto& section : sections )
        result.append( SectionWrapper( section, morphology ));
    return result;
}

SectionWrapper Morphology_getSection( const MorphologyPtr& morphology,
                                      const uint32_t id )
{
    return SectionWrapper( morphology->getSection( id ), morphology );
}

SomaWrapper Morphology_getSoma( const MorphologyPtr& morphology )
{
    return SomaWrapper( morphology->getSoma(), morphology );
}

bp::object Morphology_getTransformation( const MorphologyPtr& morphology )
{
    return toNumpy( morphology->getTransformation( ));
}

}

void export_Morphology()
{

bp::class_< SomaWrapper >(
    "Soma", bp::no_init )
    .def( "profile_points", Soma_getProfilePoints )
    .def( "mean_radius", &Soma::getMeanRadius )
    .def( "centroid", &Soma::getCentroid )
    ;

bp::class_< SectionWrapper >(
    "Section", bp::no_init )
    .def( bp::self == bp::self )
    .def( "id", &Section::getID )
    .def( "type", &Section::getType )
    .def( "length", &Section::getLength )
    .def( "samples", Section_getSamples )
    .def( "samples", Section_getSamplesFromPositions )
    .def( "distance_to_soma", &Section::getDistanceToSoma )
    .def( "sample_distances_to_soma", Section_getSampleDistancesToSoma )
    .def( "parent", Section_getParent )
    .def( "children", Section_getChildren )
    ;

bp::class_< Morphology, boost::noncopyable, MorphologyPtr >(
    "Morphology", bp::no_init )
    .def( "__init__", bp::make_constructor( Morphology_initFromURI ))
    .def( "__init__", bp::make_constructor( Morphology_initFromURIAndTransform ))
    .def( "points", Morphology_getPoints )
    .def( "sections", Morphology_getSections )
    .def( "section_types", Morphology_getSectionTypes)
    .def( "apicals", Morphology_getApicals )
    .def( "section_ids", Morphology_getSectionIDs )
    .def( "sections", Morphology_getSectionsByType )
    .def( "section", Morphology_getSection )
    .def( "soma", Morphology_getSoma )
    .def( "transformation", Morphology_getTransformation )
    ;

}

}
}
