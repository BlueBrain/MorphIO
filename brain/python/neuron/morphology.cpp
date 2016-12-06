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
#include "docstrings.h"

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
    return MorphologyPtr( new Morphology( URI( uri ),
                                          fromNumpy< Matrix4f >( transform )));
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

const auto selfarg = bp::arg( "self" );

// Do not modify whitespace on DOXY_FN lines

bp::class_< SomaWrapper >(
    "Soma", DOXY_CLASS( brain::neuron::Soma ), bp::no_init )
    .def( "profile_points", Soma_getProfilePoints, ( selfarg ),
          DOXY_FN( brain::neuron::Soma::getProfilePoints ))
    .def( "mean_radius", &Soma::getMeanRadius, ( selfarg ),
          DOXY_FN( brain::neuron::Soma::getMeanRadius ))
    .def( "centroid", &Soma::getCentroid, ( selfarg ),
          DOXY_FN( brain::neuron::Soma::getCentroid ))
    ;

bp::class_< SectionWrapper >(
    "Section", DOXY_CLASS( brain::neuron::Section ), bp::no_init )
    .def( bp::self == bp::self )
    .def( "id", &Section::getID, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getID ))
    .def( "type", &Section::getType, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getType ))
    .def( "length", &Section::getLength, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getLength ))
    .def( "samples", Section_getSamples, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getSamples() const))
    .def( "samples", Section_getSamplesFromPositions,
          ( selfarg, bp::arg( "positions" )),
          DOXY_FN( brain::neuron::Section::getSamples(const floats&) const))
    .def( "distance_to_soma", &Section::getDistanceToSoma, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getDistanceToSoma ))
    .def( "sample_distances_to_soma", Section_getSampleDistancesToSoma,
          ( selfarg ),
          DOXY_FN( brain::neuron::Section::getSampleDistancesToSoma ))
    .def( "parent", Section_getParent, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getParent ))
    .def( "children", Section_getChildren, ( selfarg ),
          DOXY_FN( brain::neuron::Section::getChildren ))
    ;

bp::class_< Morphology, boost::noncopyable, MorphologyPtr >(
    "Morphology", DOXY_CLASS( brain::neuron::Morphology ), bp::no_init )
    .def( "__init__", bp::make_constructor( Morphology_initFromURI ),
          DOXY_FN( brain::neuron::Morphology::Morphology(const URI&)))
    .def( "__init__", bp::make_constructor( Morphology_initFromURIAndTransform ),
          DOXY_FN( brain::neuron::Morphology::Morphology(const URI&, const Matrix4f&)))
    // The following docstrings are given explictly because the return types
    // are special and the original documentation uses @sa, which points nowhere.
    .def( "points", Morphology_getPoints, ( selfarg ),
          "Return a 4xN numpy array with the x,y,z and radius of all the points of this morphology.")
    .def( "sections", Morphology_getSections, ( selfarg ),
          "Return a 2xN numpy array with the parent ID and first point offset of each section." )
    .def( "section_types", Morphology_getSectionTypes, ( selfarg ),
          "Return a numpy array with the section types." )
    .def( "apicals", Morphology_getApicals, ( selfarg ),
          "Return a 2xN numpy array with the section id and point index of apical points." )
    .def( "section_ids", Morphology_getSectionIDs,
          ( selfarg, bp::arg( "types" )),
          DOXY_FN( brain::neuron::Morphology::getSectionIDs ))
    .def( "sections", Morphology_getSectionsByType,
          ( selfarg, bp::arg( "type" )),
          DOXY_FN( brain::neuron::Morphology::getSections(const SectionTypes&) const))
    .def( "section", Morphology_getSection, ( selfarg, bp::arg( "id" )),
          DOXY_FN( brain::neuron::Morphology::getSections(SectionType) const))
    .def( "soma", Morphology_getSoma, ( selfarg ),
          DOXY_FN( brain::neuron::Morphology::getSoma ))
    .def( "transformation", Morphology_getTransformation, ( selfarg ),
          DOXY_FN( brain::neuron::Morphology::getTransformation ))
    ;

}

}
}
