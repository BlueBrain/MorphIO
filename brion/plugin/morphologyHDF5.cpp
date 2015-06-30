/* Copyright (c) 2013-2015, EPFL/Blue Brain Project
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

#include "morphologyHDF5.h"

#include "../detail/lockHDF5.h"
#include "../detail/silenceHDF5.h"

#include <brion/version.h>

#include <lunchbox/debug.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/pluginRegisterer.h>

#include <H5Cpp.h>

#include <boost/lexical_cast.hpp>

namespace boost
{
template<>
inline std::string lexical_cast( const brion::MorphologyRepairStage& s )
{
    switch( s )
    {
    case brion::MORPHOLOGY_RAW: return "raw";
    case brion::MORPHOLOGY_UNRAVELED: return "unraveled";
    case brion::MORPHOLOGY_REPAIRED: return "repaired";
    case brion::MORPHOLOGY_UNDEFINED:
    default:
        throw boost::bad_lexical_cast();
    }
}
}

namespace brion
{
namespace plugin
{

namespace
{

// v1 & v2
const std::string _d_points( "points" );
const size_t _pointColumns = 4;

// v1
const std::string _d_structure( "/structure" );
const size_t _structureV1Columns = 3;

// v2
const std::string _g_structure( "structure" );
const size_t _structureV2Columns = 2;
const std::string _g_root( "neuron1" );
const std::string _d_type( "sectiontype" );
const std::string _a_apical( "apical" );
const std::string _a_creator( "creator" );
const std::string _a_version( "version" );

template< typename T > inline std::string toString( const T& t )
{
    return boost::lexical_cast< std::string >( t );
}


lunchbox::PluginRegisterer< MorphologyHDF5 > registerer;

}


#define ASSERT_WRITE                                                         \
    if( !_write )                                                            \
        LBTHROW( std::runtime_error( "Cannot not write read-only morphology" \
                                     " file " + _file.getFileName( )));

MorphologyHDF5::MorphologyHDF5( const MorphologyInitData& initData )
    : _file()
    , _version( MORPHOLOGY_VERSION_H5_2 )
    , _stage( MORPHOLOGY_UNDEFINED )
    , _write( false )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    const std::string path = initData.getURI().getPath();
    unsigned int flags = 0;
    switch( initData.getAccessMode( ))
    {
    case MODE_READ:
        flags = H5F_ACC_RDONLY;
        _write = false;
        break;
    case MODE_WRITE:
        _write = true;
        flags = H5F_ACC_EXCL;
        break;
    case MODE_OVERWRITE:
        _write = true;
        flags = H5F_ACC_TRUNC;
        break;
    default:
        LBTHROW( std::runtime_error( "Could not open morphology file " +
                                     path + ": Invalid access mode" ));
    }

    try
    {
        detail::SilenceHDF5 silence;
        if( _write )
            _file = H5::H5File( path, flags );
        else
            _file.openFile( path, flags );
    }
    catch( const H5::Exception& exc )
    {
        LBTHROW( std::runtime_error( _write ?
                                     "Could not create morphology file " :
                                     "Could not open morphology file " +
                                     path + ": " + exc.getDetailMsg( )));
    }

    if( _write )
    {
        _version = initData.getVersion();
        switch( _version )
        {
        case MORPHOLOGY_VERSION_H5_1:
            _writeV1Header();
            break;
        case MORPHOLOGY_VERSION_UNDEFINED:
            _version = MORPHOLOGY_VERSION_H5_2;
            // no break;
        case MORPHOLOGY_VERSION_H5_2:
            _writeV2Header();
            break;
        default:
            LBTHROW( std::runtime_error( "Unknown morphology file format for "
                                         "file " + path ));
        }
    }
    else
    {
        _checkVersion( path );
        _selectRepairStage();
    }
}

MorphologyHDF5::~MorphologyHDF5()
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    if( _file.getId( ))
        _file.close();
}

bool MorphologyHDF5::handles( const MorphologyInitData& initData )
{
    const std::string path = initData.getURI().getPath();
    const size_t pos = path.find_last_of( "." );
    if( pos == std::string::npos )
        return false;

    return path.substr( pos ) == ".h5";
}

Vector4fsPtr MorphologyHDF5::readPoints( MorphologyRepairStage stage ) const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    H5::DataSet dataset;
    try
    {
        if( _version == MORPHOLOGY_VERSION_H5_1 )
            dataset = _file.openDataSet( "/" + _d_points );
        else
        {
            if( stage == MORPHOLOGY_UNDEFINED )
                stage = _stage;
            dataset = _file.openDataSet(
                "/" + _g_root + "/" + toString( stage ) + "/" + _d_points );
        }
    }
    catch( ... )
    {
        LBERROR << "Could not open points dataset for morphology file "
                << _file.getFileName() << " repair stage "
                << toString( stage ) << std::endl;
        return Vector4fsPtr( new Vector4fs );
    }

    hsize_t dims[2];
    const H5::DataSpace& dspace = dataset.getSpace();
    if( dspace.getSimpleExtentNdims() != 2 ||
        dspace.getSimpleExtentDims( dims ) < 0 || dims[1] != _pointColumns )
    {
        LBTHROW( std::runtime_error( "Reading morphology file '" +
                      _file.getFileName() + "': bad number of dimensions in"
                      " 'points' dataspace"));
    }

    Vector4fsPtr data( new Vector4fs( dims[0] ));
    dataset.read( data->data(), H5::PredType::NATIVE_FLOAT );
    return data;
}

Vector2isPtr MorphologyHDF5::readSections( MorphologyRepairStage stage ) const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    if( _version == MORPHOLOGY_VERSION_H5_1 )
    {
        const H5::DataSet& dataset = _file.openDataSet( _d_structure );

        hsize_t dims[2];
        const H5::DataSpace& dspace  = dataset.getSpace();
        if( dspace.getSimpleExtentNdims() != 2 ||
            dspace.getSimpleExtentDims( dims ) < 0 ||
            dims[1] != _structureV1Columns )
        {
            LBTHROW( std::runtime_error( "Reading morphology file '" +
                      _file.getFileName() + "': bad number of dimensions in"
                      " 'structure' dataspace"));
        }

        const hsize_t readCount[2] = { dims[0], 1 };
        const hsize_t readOffset[2] = { 0, 1 };
        dspace.selectHyperslab( H5S_SELECT_XOR, readCount, readOffset );

        Vector2isPtr data( new Vector2is( dims[0] ));
        const hsize_t mdim[2] = { dims[0], 2 };
        const H5::DataSpace mspace( 2, mdim );
        dataset.read( data->data(), H5::PredType::NATIVE_INT, mspace,
                      dspace );
        return data;
    }

    if( stage == MORPHOLOGY_UNDEFINED )
        stage = _stage;

    // fixes BBPSDK-295 by restoring old BBPSDK 0.13 implementation
    if( stage == MORPHOLOGY_UNRAVELED )
        stage = MORPHOLOGY_RAW;

    H5::DataSet dataset;
    try
    {
        dataset = _file.openDataSet( "/" + _g_root  + "/" + _g_structure +
                                     "/" + toString( stage ));
    }
    catch( ... )
    {
        LBERROR << "Could not open sections dataset for morphology file "
                << _file.getFileName() << " repair stage "
                << toString( stage ) << std::endl;
        return Vector2isPtr( new Vector2is );
    }

    hsize_t dims[2];
    const H5::DataSpace& dspace  = dataset.getSpace();
    if( dspace.getSimpleExtentNdims() != 2 ||
        dspace.getSimpleExtentDims( dims ) < 0 ||
        dims[1] != _structureV2Columns )
    {
        LBTHROW( std::runtime_error( "Reading morphology file '" +
                  _file.getFileName() + "': bad number of dimensions in"
                  " 'structure' dataspace"));
    }

    Vector2isPtr data( new Vector2is( dims[0] ));
    dataset.read( data->data(), H5::PredType::NATIVE_INT );
    return data;
}

SectionTypesPtr MorphologyHDF5::readSectionTypes() const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    if( _version == MORPHOLOGY_VERSION_H5_1 )
    {
        const H5::DataSet& dataset = _file.openDataSet( _d_structure );

        hsize_t dims[2];
        const H5::DataSpace& dspace  = dataset.getSpace();
        if( dspace.getSimpleExtentNdims() != 2 ||
            dspace.getSimpleExtentDims( dims ) < 0 ||
            dims[1] != _structureV1Columns )
        {
            LBTHROW( std::runtime_error( "Reading morphology file '" +
                      _file.getFileName() + "': bad number of dimensions in"
                      " 'structure' dataspace"));
        }

        const hsize_t readCount[2] = { dims[0], 1 };
        const hsize_t readOffset[2] = { 0, 1 };
        dspace.selectHyperslab( H5S_SELECT_SET, readCount, readOffset );

        SectionTypesPtr data( new SectionTypes( dims[0] ));
        const hsize_t mdim = dims[0];
        const H5::DataSpace mspace( 1, &mdim );
        dataset.read( data->data(), H5::PredType::NATIVE_INT, mspace,
                      dspace );
        return data;
    }

    const H5::DataSet& dataset = _file.openDataSet( "/" + _g_root  +
                                 "/" + _g_structure + "/" + _d_type );

    hsize_t dims[2];
    const H5::DataSpace& dspace  = dataset.getSpace();
    if( dspace.getSimpleExtentNdims() != 2 ||
        dspace.getSimpleExtentDims( dims ) < 0 || dims[1] != 1 )
    {
        LBTHROW( std::runtime_error( "Reading morphology file '" +
                  _file.getFileName() + "': bad number of dimensions in"
                  " 'sectiontype' dataspace"));
    }

    SectionTypesPtr data( new SectionTypes( dims[0] ));
    dataset.read( data->data(), H5::PredType::NATIVE_INT );
    return data;
}

Vector2isPtr MorphologyHDF5::readApicals() const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    Vector2isPtr data( new Vector2is );
    if( _version == MORPHOLOGY_VERSION_H5_1 )
        return data;

    try
    {
        detail::SilenceHDF5 silence;
        const H5::Group& root = _file.openGroup( _g_root );
        const H5::Attribute& attr = root.openAttribute( _a_apical );
        const H5::DataSpace dspace = attr.getSpace();

        hsize_t dims[1];
        dspace.getSimpleExtentDims( dims );

        data->resize( dims[0] / 2 );
        attr.read( H5::PredType::NATIVE_INT, data->data( ));
    }
    catch( ... ) {}

    return data;
}

MorphologyVersion MorphologyHDF5::getVersion() const
{
    return _version;
}

void MorphologyHDF5::writePoints( const Vector4fs& points,
                                  const MorphologyRepairStage stage )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;

    H5::FloatType pointsDT( H5::PredType::NATIVE_DOUBLE );
    pointsDT.setOrder( H5T_ORDER_LE );
    hsize_t dim[2] = { points.size(), 4 };
    H5::DataSpace pointsDS( 2, dim );

    H5::DataSet dataset;
    if( _version == MORPHOLOGY_VERSION_H5_1 )
         dataset = _file.createDataSet( _d_points, pointsDT, pointsDS );
    else
    {
        H5::Group root = _file.openGroup( _g_root );
        H5::Group group = root.createGroup( toString( stage ));
        dataset = group.createDataSet( _d_points, pointsDT, pointsDS );
    }

    dataset.write( points.data(), H5::PredType::NATIVE_FLOAT );
}

void MorphologyHDF5::writeSections( const Vector2is& sections,
                                    const MorphologyRepairStage stage )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;

    H5::FloatType structureDT( H5::PredType::NATIVE_INT );
    structureDT.setOrder( H5T_ORDER_LE );

    if( _version == MORPHOLOGY_VERSION_H5_1 )
    {
        hsize_t dim[2] = { sections.size(), 3 };
        H5::DataSpace structureDS( 2, dim );

        H5::DataSet dataset = _file.getNumObjs() == 2 ?
                    _file.openDataSet( _g_structure ) :
              _file.createDataSet( _d_structure, structureDT, structureDS );

        const H5::DataSpace& dspace = dataset.getSpace();
        const hsize_t count[2] = { sections.size(), 1 };
        const hsize_t offset[2] = { 0, 1 };
        dspace.selectHyperslab( H5S_SELECT_XOR, count, offset );

        const hsize_t mdim[2] = { sections.size(), 2 };
        const H5::DataSpace mspace( 2, mdim );
        dataset.write( sections.data(), H5::PredType::NATIVE_INT, mspace,
                       dspace );
        return;
    }

    hsize_t dim[2] = { sections.size(), 2 };
    H5::DataSpace structureDS( 2, dim );

    H5::Group root = _file.openGroup( _g_root + "/" + _g_structure );
    H5::DataSet dataset = root.createDataSet( toString( stage ),
                                              structureDT, structureDS );
    dataset.write( sections.data(), H5::PredType::NATIVE_INT );
}

void MorphologyHDF5::writeSectionTypes( const SectionTypes& types )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;

    H5::FloatType structureDT( H5::PredType::NATIVE_INT );
    structureDT.setOrder( H5T_ORDER_LE );

    if( _version == MORPHOLOGY_VERSION_H5_1 )
    {
        hsize_t dim[2] = { types.size(), 3 };
        H5::DataSpace structureDS( 2, dim );

        H5::DataSet dataset = _file.getNumObjs() == 2 ?
                    _file.openDataSet( _g_structure ) :
              _file.createDataSet( _d_structure, structureDT, structureDS );

        const H5::DataSpace& dspace = dataset.getSpace();
        const hsize_t count[2] = { types.size(), 1 };
        const hsize_t offset[2] = { 0, 1 };
        dspace.selectHyperslab( H5S_SELECT_SET, count, offset );

        const hsize_t mdim = types.size();
        const H5::DataSpace mspace( 1, &mdim );
        dataset.write( types.data(), H5::PredType::NATIVE_INT, mspace,
                       dspace );
        return;
    }

    hsize_t dim[2] = { types.size(), 1 };
    H5::DataSpace structureDS( 2, dim );

    H5::Group root = _file.openGroup( _g_root + "/" + _g_structure );
    H5::DataSet dataset = root.createDataSet( _d_type, structureDT,
                                              structureDS );
    dataset.write( types.data(), H5::PredType::NATIVE_INT );
}

void MorphologyHDF5::writeApicals( const Vector2is& apicals )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;

    if( _version == MORPHOLOGY_VERSION_H5_1 )
        LBTHROW( std::runtime_error( "No apical sections for version 1 "
                                     "morphology files." ));

    if( apicals.empty( ))
        return;

    const H5::Group& root = _file.openGroup( _g_root );
    hsize_t dims = 2 * apicals.size();
    H5::DataSpace apicalDS( 1, &dims );
    H5::Attribute attr = root.createAttribute( _a_apical,
                                    H5::PredType::NATIVE_DOUBLE, apicalDS );
    attr.write( H5::PredType::NATIVE_INT, apicals.data( ));
}

void MorphologyHDF5::flush()
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;
    _file.flush( H5F_SCOPE_GLOBAL );
}

void MorphologyHDF5::_checkVersion( const std::string& source )
{
    if( _isV1( ))
        _version = MORPHOLOGY_VERSION_H5_1;
    else if( _isV2( ))
        _version = MORPHOLOGY_VERSION_H5_2;
    else
        LBTHROW( std::runtime_error( "Unknown morphology file format for "
                                     "file " + source ));
}

void MorphologyHDF5::_selectRepairStage()
{
    if( _version == MORPHOLOGY_VERSION_H5_1 )
        return;

    MorphologyRepairStage stages[3] = { MORPHOLOGY_REPAIRED,
                                        MORPHOLOGY_UNRAVELED,
                                        MORPHOLOGY_RAW };
    for( size_t i = 0; i < 3; ++i )
    {
        try
        {
            detail::SilenceHDF5 silence;
            _file.openDataSet( "/" + _g_root + "/" +
                               toString( stages[i] ) + "/" + _d_points );
            _stage = stages[i];
            break;
        }
        catch( const H5::Exception& ) {}
    }
}

bool MorphologyHDF5:: _isV1() const
{
    try
    {
        detail::SilenceHDF5 silence;
        _file.openDataSet( _d_structure );
        return true;
    }
    catch( const H5::Exception& )
    {
        return false;
    }
}

bool MorphologyHDF5:: _isV2() const
{
    try
    {
        detail::SilenceHDF5 silence;
        const H5::Group& root = _file.openGroup( _g_root );
        const H5::Attribute& attr = root.openAttribute( _a_version );

        int32_t version;
        attr.read( H5::PredType::NATIVE_INT, &version );

        if( version == MORPHOLOGY_VERSION_H5_2 )
            return true;
    }
    catch( const H5::Exception& ) {}

    try
    {
        detail::SilenceHDF5 silence;
        _file.openGroup( _g_root );
        return true;
    }
    catch( const H5::Exception& )
    {
        return false;
    }
}

void MorphologyHDF5::_writeV1Header()
{
    ASSERT_WRITE;
}

void MorphologyHDF5::_writeV2Header()
{
    ASSERT_WRITE;

    H5::Group root = _file.createGroup( _g_root );
    root.createGroup( _g_structure );

    const time_t now = ::time(0);
#ifdef _WIN32
    char* gmtString = ::ctime( &now );
#else
    char gmtString[32];
    ::ctime_r( &now, gmtString );
#endif
    const std::string creator = "Brion " + Version::getString() +
                                " brion::Morphology " + gmtString;
    H5::DataSpace creatorDS( H5S_SCALAR );
    H5::StrType stringDT( H5::PredType::C_S1, creator.length( ));
    H5::Attribute creatorAttr = root.createAttribute( _a_creator, stringDT,
                                                      creatorDS );
    creatorAttr.write( stringDT, creator );

    H5::DataSpace versionDS( H5S_SCALAR );
    H5::Attribute versionAttr = root.createAttribute( _a_version,
                                      H5::PredType::NATIVE_INT, versionDS );
    const int version = MORPHOLOGY_VERSION_H5_2;
    versionAttr.write( H5::PredType::NATIVE_INT, &version );
}

}
}
