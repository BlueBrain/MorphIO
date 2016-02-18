/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include "circuit.h"
#include "neuron/morphology.h"

#include <cassert>
#include <algorithm>
#include <functional>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/math/constants/constants.hpp>

#include <brion/blueConfig.h>
#include <brion/circuit.h>
#include <brion/morphology.h>
#include <brion/target.h>

#include <mvd/mvd3.hpp>
#include <mvd/mvd_generic.hpp>

#include <lunchbox/log.h>



namespace brain
{


Vector3fs getPositionsMVD2(const brion::Circuit& circuit_mvd2, const brion::GIDSet & gids);
Vector3fs getPositionsMVD3(MVD3::MVD3File& circuit_mvd3, const brion::GIDSet & gids);

std::vector<std::string> getMorphologiesNameMVD2(const brion::Circuit& circuit_mvd2, const brion::GIDSet & gids);
std::vector<std::string> getMorphologiesNameMVD3(MVD3::MVD3File& circuit_mvd3, const brion::GIDSet & gids);

Quaternionfs getQuatRotationsMVD2(const brion::Circuit& circuit_mvd2, const GIDSet& gids );
Quaternionfs getQuatRotationsMVD3(MVD3::MVD3File& circuit_mvd3, const GIDSet& gids );

class Circuit::Impl
{
public:

    Impl( const brion::BlueConfig& config )
        : _circuit_mvd2()
        , _circuit_mvd3()
        , _morphologySource( config.getMorphologySource( ))
        , _circuitTarget( config.getCircuitTarget( ))
        , _targetParsers( config.getTargets( ))
    {
        using namespace MVD;
        const std::string circuit_path = config.getCircuitSource().getPath();
        switch(is_mvd_file(circuit_path))
        {
            case MVDType::MVD2:
                _circuit_mvd2.reset(new brion::Circuit(circuit_path));
                break;
            case MVDType::MVD3:
            default:
                _circuit_mvd3.reset(new MVD3::MVD3File(circuit_path));
                break;
        }
    }

    GIDSet getGIDs( const std::string& target ) const
    {
        return brion::Target::parse(
            _targetParsers, target.empty() ? _circuitTarget : target );
    }

    Vector3fs getPositions(const GIDSet& gids) const{
        if(_circuit_mvd2)
            return getPositionsMVD2(*_circuit_mvd2, gids);
        return getPositionsMVD3(*_circuit_mvd3, gids);

    }

    Quaternionfs getQuatRot(const GIDSet & gids) const{
        if(_circuit_mvd2)
            return getQuatRotationsMVD2(*_circuit_mvd2, gids);
        return getQuatRotationsMVD3(*_circuit_mvd3, gids);
    }

    std::vector<std::string> getMorphologiesName(const GIDSet& gids){
        if(_circuit_mvd2)
            return getMorphologiesNameMVD2(*_circuit_mvd2, gids);
        return getMorphologiesNameMVD3(*_circuit_mvd3, gids);
    }

    URI getMorphologyURI( const std::string& name ) const
    {
        URI uri;
        uri.setPath( _morphologySource.getPath() + "/" + name + ".h5" );
        uri.setScheme( "file" );
        return uri;
    }

private:

    boost::scoped_ptr<brion::Circuit> _circuit_mvd2;
    mutable boost::scoped_ptr<MVD3::MVD3File> _circuit_mvd3;

    const brion::URI _morphologySource;
    const std::string _circuitTarget;
    const brion::Targets _targetParsers;

};


MVD3::Range range_from_gidset(const GIDSet& gids)
{
    if(gids.size() > 0)
    {
        assert(*gids.begin() > 0); // gids in Brion start at 1
        const size_t offset = (*gids.begin()) -1;
        const size_t range_count = *gids.rbegin() - offset;
        return MVD3::Range(offset, range_count);
    }
    return MVD3::Range(0,0); // full range
}



template<typename SrcArray, typename DstArray, typename AssignOps>
inline void array_range_to_index(const MVD3::Range& range,
                                 const GIDSet& gids,
                                 SrcArray& src,
                                 DstArray& dst,
                                 const AssignOps& assign_ops)
{
    size_t s_source = std::distance(src.begin(), src.end()); (void) s_source;
    size_t s_dest = std::distance(dst.begin(), dst.end()); (void) s_dest;

    if(gids.empty() == true) // we work on full range, no translation needed
    {
        assert(s_source == s_dest);
        std::transform(src.begin(), src.end(), dst.begin(), assign_ops);
    }
    else
    {
        assert(s_dest == gids.size());
        typename DstArray::iterator dst_it = dst.begin();
        for(GIDSet::const_iterator it = gids.begin(); it != gids.end(); ++it)
        {
            assert( *it > range.offset && *it < (range.offset + range.count +1));
            typename SrcArray::iterator src_it = src.begin();
            std::advance(src_it, *it - range.offset -1 );
            *dst_it = assign_ops(*src_it);
            ++dst_it;
        }
    }
}

template<typename VMMLVector>
inline VMMLVector boost_mul_array_to_vmml_vector(const MVD3::Positions::subarray<1>::type& subarray)
{
    VMMLVector res;
    res.iter_set(subarray.begin(), subarray.end());
    return res;
}

inline brion::Quaternionf boost_mul_array_to_quaternion(const MVD3::Positions::subarray<1>::type& subarray)
{
    return brion::Quaternionf(subarray[1], subarray[2], subarray[3], subarray[0]);
}

inline std::string swap_string(std::string& in)
{
    using namespace std;
    std::string res;
    swap(res, in);
    return res;
}





Vector3fs getPositionsMVD2(const brion::Circuit& circuit_mvd2, const brion::GIDSet & gids)
{
    const brion::NeuronMatrix& data = circuit_mvd2.get(
        gids, brion::NEURON_POSITION_X | brion::NEURON_POSITION_Y |
             brion::NEURON_POSITION_Z );

    brion::GIDSet::const_iterator gid = gids.begin();
    Vector3fs positions( gids.size( ));
    #pragma omp parallel for
    for( size_t i = 0; i < gids.size(); ++i )
    {
        try
        {
            positions[i] =
                brion::Vector3f( boost::lexical_cast< float >( data[i][0] ),
                                 boost::lexical_cast< float >( data[i][1] ),
                                 boost::lexical_cast< float >( data[i][2] ));
        }
        catch( const boost::bad_lexical_cast& )
        {
            LBWARN << "Error parsing circuit position or orientation for gid "
                   << *gid << ". Morphology not transformed." << std::endl;
            positions[i] = Vector3f::ZERO;
        }
        #pragma omp critical (brain_circuit_getPositions)
        ++gid;
    }
    return positions;
}


Vector3fs getPositionsMVD3(MVD3::MVD3File& circuit_mvd3, const brion::GIDSet & gids)
{
    const size_t n_elems = ((gids.size()>0)?gids.size():circuit_mvd3.getNbNeuron());
    Vector3fs results(n_elems);
    MVD3::Range range = range_from_gidset(gids);
    MVD3::Positions positions = circuit_mvd3.getPositions(range);

    assert(positions.shape()[1] ==3);
    array_range_to_index(range, gids, positions, results, boost_mul_array_to_vmml_vector<Vector3f>);
    return results;

}

std::vector<std::string> getMorphologiesNameMVD2(const brion::Circuit& circuit_mvd2, const brion::GIDSet & gids)
{
    brion::NeuronMatrix matrix =
        circuit_mvd2.get( gids, brion::NEURON_MORPHOLOGY_NAME );
    std::vector<std::string> res(matrix.shape()[0]);

    brion::NeuronMatrix::array_view<1>::type morpho_view = matrix[ boost::indices[brion::NeuronMatrix::index_range()][0]];
    std::transform(morpho_view.begin(), morpho_view.end(), res.begin(), swap_string);
    return res;
}


std::vector<std::string> getMorphologiesNameMVD3(MVD3::MVD3File& circuit_mvd3, const brion::GIDSet & gids)
{
    const size_t n_elems = ((gids.size()>0)?gids.size():circuit_mvd3.getNbNeuron());
    std::vector<std::string> results(n_elems);
    MVD3::Range range = range_from_gidset(gids);
    std::vector<std::string> morphos = circuit_mvd3.getMorphologies(range);

    assert(morphos.size() == n_elems);
    array_range_to_index(range, gids, morphos, results, swap_string);
    return results;

}


Quaternionfs getQuatRotationsMVD2(const brion::Circuit& circuit_mvd2, const GIDSet& gids )
{
    const double deg_rad = boost::math::constants::pi<double>() / 180.0;
    const size_t n_elems = (gids.size()>0)?gids.size():circuit_mvd2.getNumNeurons();
    const brion::NeuronMatrix& data = circuit_mvd2.get(
        gids, brion::NEURON_ROTATION );
    Quaternionfs rotations(n_elems);

    assert(data.shape()[0] == n_elems);
    assert(data.shape()[1] == 1);

    #pragma omp parallel for
    for( size_t i = 0; i < n_elems; i++ )
    {
        try
        {
            // transform rotation Y angle in degree into rotation quaternion
            const double angle_y = boost::lexical_cast<double>( data[i][0] )*deg_rad;
            rotations[i] =
                Quaternionf( 0,                     // x
                             std::sin(angle_y/2),   // y
                             0,                     // z
                             std::cos(angle_y/2));  // w
        }
        catch( const boost::bad_lexical_cast& )
        {
            GIDSet::const_iterator it_gid = gids.begin();
            std::advance(it_gid, i);
            LBWARN << "Error parsing circuit position or orientation for gid "
                   << *(it_gid) << ". Morphology not transformed." << std::endl;
            rotations[i] = Quaternionf(0,0,0,0);
        }
    }
    return rotations;
 }

Quaternionfs getQuatRotationsMVD3(MVD3::MVD3File& circuit_mvd3, const GIDSet& gids )
{
    const size_t n_elems = ((gids.size() >0)?gids.size():circuit_mvd3.getNbNeuron());
    Quaternionfs results(n_elems);

    MVD3::Range range = range_from_gidset(gids);
    MVD3::Rotations rotations = circuit_mvd3.getRotations(range);
    assert(rotations.shape()[1] ==4);
    array_range_to_index(range, gids, rotations, results, boost_mul_array_to_quaternion);
    return results;

 }




Circuit::Circuit( const URI& source )
  : _impl( new Impl( brion::BlueConfig( source.getPath( ))))
{
}

Circuit::Circuit( const brion::BlueConfig& config )
  : _impl( new Impl( config ))
{
}

Circuit::~Circuit()
{
    delete _impl;
}

GIDSet Circuit::getGIDs( const std::string& target ) const
{
    return _impl->getGIDs( target );
}

URIs Circuit::getMorphologyURIs( const GIDSet& gids ) const
{
    const std::vector<std::string> names = _impl->getMorphologiesName(gids);

    URIs uris;
    uris.reserve( names.size());
    for(std::vector<std::string>::const_iterator it = names.begin(); it < names.end(); ++it )
        uris.push_back( _impl->getMorphologyURI( *it ));
    return uris;
}

neuron::Morphologies Circuit::loadMorphologies( const GIDSet& gids,
                                        const Coordinates coords ) const
{
    const URIs& uris = getMorphologyURIs( gids );
    neuron::Morphologies result;
    result.reserve( uris.size( ));

    if( coords == COORDINATES_GLOBAL )
    {
        const Matrix4fs& transforms = getTransforms( gids );
        for( size_t i = 0; i < uris.size(); ++i )
        {
            const URI& uri = uris[i];
            const brion::Morphology raw( uri.getPath( ));
            result.push_back( neuron::MorphologyPtr(
                                 new neuron::Morphology( raw, transforms[i] )));
        }
        return result;
    }

    std::map< std::string, neuron::MorphologyPtr > loaded;
    for( size_t i = 0; i < uris.size(); ++i )
    {
        const URI& uri = uris[i];

        neuron::MorphologyPtr& morphology = loaded[uri.getPath()];
        if( !morphology )
        {
            const brion::Morphology raw( uri.getPath( ));
            morphology.reset( new neuron::Morphology( raw ));
        }

        result.push_back( morphology );
    }
    return result;
}




Vector3fs Circuit::getPositions( const GIDSet& gids ) const
{
    return _impl->getPositions(gids);
}


Matrix4fs Circuit::getTransforms( const GIDSet& gids ) const
{
    const Vector3fs positions = _impl->getPositions(gids);
    const Quaternionfs rotations = _impl->getQuatRot(gids);
    assert(positions.size() == rotations.size());

    Matrix4fs transforms( positions.size(), Matrix4f::IDENTITY );

    #pragma omp parallel for
    for( size_t i = 0; i < positions.size(); ++i )
    {
        Matrix4f& matrix = transforms[i];

        rotations[i].get_rotation_matrix(matrix);
        matrix.set_translation(positions[i]);
    }
    return transforms;

}


}
