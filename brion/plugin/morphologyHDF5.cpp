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

#include "morphologyHDF5.h"

#include "../detail/lockHDF5.h"
#include "../detail/silenceHDF5.h"
#include "../detail/utilsHDF5.h"

#include <brion/version.h>

#include <lunchbox/debug.h>
#include <lunchbox/scopedMutex.h>
#include <lunchbox/pluginRegisterer.h>

#include <H5Cpp.h>

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

// v1.1
const std::string _g_metadata( "/metadata" );
const std::string _e_family( "cell_family_enum" );
const std::string _a_family( "cell_family" );
const std::string _d_perimeters( "/perimeters" );
const std::string _a_software_version( "software_version" );
const std::string _a_creation_time( "creation_time" );

// v1.1 & v2
const std::string _a_creator( "creator" );
const std::string _a_version( "version" );

// v2
const std::string _g_structure( "structure" );
const size_t _structureV2Columns = 2;
const std::string _g_root( "neuron1" );
const std::string _d_type( "sectiontype" );
const std::string _a_apical( "apical" );


std::string toString( const brion::enums::MorphologyRepairStage& s )
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

lunchbox::PluginRegisterer< MorphologyHDF5 > registerer;

}


#define ASSERT_WRITE                                                         \
    if( !_write )                                                            \
        LBTHROW( std::runtime_error( "Cannot not write read-only morphology" \
                                     " file " + _file.getFileName( )));

MorphologyHDF5::MorphologyHDF5( const MorphologyInitData& initData )
    : _file()
    , _version( MORPHOLOGY_VERSION_UNDEFINED )
    , _stage( MORPHOLOGY_UNDEFINED )
    , _family( FAMILY_NEURON )
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
            // no metadata for version 1
            break;
        case MORPHOLOGY_VERSION_H5_1_1:
            _writeV11Metadata( initData );
            break;
        case MORPHOLOGY_VERSION_UNDEFINED:
            _version = MORPHOLOGY_VERSION_H5_2;
            // no break;
        case MORPHOLOGY_VERSION_H5_2:
            _writeV2Metadata();
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

    if( _points.getId( ))
        _points.close();

    if( _sections.getId( ))
        _sections.close();

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

std::string MorphologyHDF5::getDescription()
{
    return "Blue Brain hdf5 morphologies:\n"
           "  [file://]/path/to/morphology.h5";
}

CellFamily MorphologyHDF5::getCellFamily() const
{
    return _family;
}

Vector4fsPtr MorphologyHDF5::readPoints( MorphologyRepairStage stage ) const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    if( _version == MORPHOLOGY_VERSION_H5_2 )
    {
        H5::DataSet dataset;
        try
        {
            if( stage == MORPHOLOGY_UNDEFINED )
                stage = _stage;
            dataset = _file.openDataSet(
                "/" + _g_root + "/" + toString( stage ) + "/" + _d_points );
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

    Vector4fsPtr data( new Vector4fs( _pointsDims[0] ));
    _points.read( data->data(), H5::PredType::NATIVE_FLOAT );
    return data;
}

Vector2isPtr MorphologyHDF5::readSections( MorphologyRepairStage stage ) const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    if( _version == MORPHOLOGY_VERSION_H5_2 )
    {
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

    const hsize_t readCount[2] = { _sectionsDims[0], 1 };
    const hsize_t readOffset[2] = { 0, 1 };
    H5::DataSpace dspace = _sections.getSpace();
    dspace.selectHyperslab( H5S_SELECT_XOR, readCount, readOffset );

    Vector2isPtr data( new Vector2is( _sectionsDims[0] ));
    const hsize_t mdim[2] = { _sectionsDims[0], 2 };
    const H5::DataSpace mspace( 2, mdim );
    _sections.read( data->data(), H5::PredType::NATIVE_INT, mspace,
                    dspace );
    return data;
}

SectionTypesPtr MorphologyHDF5::readSectionTypes() const
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    if( _version == MORPHOLOGY_VERSION_H5_2 )
    {
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

    const hsize_t readCount[2] = { _sectionsDims[0], 1 };
    const hsize_t readOffset[2] = { 0, 1 };
    H5::DataSpace dspace = _sections.getSpace();
    dspace.selectHyperslab( H5S_SELECT_SET, readCount, readOffset );

    SectionTypesPtr data( new SectionTypes( _sectionsDims[0] ));
    const hsize_t mdim = _sectionsDims[0];
    const H5::DataSpace mspace( 1, &mdim );
    _sections.read( data->data(), H5::PredType::NATIVE_INT, mspace,
                    dspace );
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

floatsPtr MorphologyHDF5::readPerimeters() const
{
    if( _version != MORPHOLOGY_VERSION_H5_1_1 )
        return floatsPtr( new floats( ));

    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    try
    {
        detail::SilenceHDF5 silence;
        H5::DataSet dataset = _file.openDataSet( _d_perimeters );

        hsize_t dims;
        const H5::DataSpace& dspace = dataset.getSpace();
        if( dspace.getSimpleExtentNdims() != 1 ||
            dspace.getSimpleExtentDims( &dims ) < 0 )
        {
            LBTHROW( std::runtime_error( "Reading morphology file '" +
                          _file.getFileName() + "': bad number of dimensions in"
                          " 'perimeters' dataspace"));
        }

        floatsPtr data( new floats( dims ));
        dataset.read( data->data(), H5::PredType::NATIVE_FLOAT );
        return data;
    }
    catch( ... )
    {
        if( getCellFamily() == FAMILY_GLIA )
            LBTHROW( std::runtime_error( "No empty perimeters allowed for glia "
                                         "morphology" ));
        return floatsPtr( new floats( ));
    }
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

    hsize_t dim[2] = { points.size(), 4 };
    H5::DataSpace pointsDS( 2, dim );

    H5::DataSet dataset;
    H5::FloatType pointsDT( H5::PredType::NATIVE_DOUBLE );
    pointsDT.setOrder( H5T_ORDER_LE );
    if( _version == MORPHOLOGY_VERSION_H5_2 )
    {
        H5::Group root = _file.openGroup( _g_root );
        H5::Group group = root.createGroup( toString( stage ));
        try
        {
            detail::SilenceHDF5 silence;
            group.openDataSet( _d_points );
            LBTHROW( std::runtime_error( "Points were already written" ));
        }
        catch( const H5::Exception& ) {}
        dataset = group.createDataSet( _d_points, pointsDT, pointsDS );
    }
    else
    {
        try
        {
            detail::SilenceHDF5 silence;
            _file.openDataSet( _d_points );
            LBTHROW( std::runtime_error( "Points were already written" ));
        }
        catch( const H5::Exception& ) {}

        try
        {
            detail::SilenceHDF5 silence;
            H5::DataSet perimeters = _file.openDataSet( _d_perimeters );
            hsize_t dims[2];
            perimeters.getSpace().getSimpleExtentDims( dims );
            if( dims[0] != points.size( ))
            {
                std::stringstream msg;
                msg << "Number of points does not match number of perimeters, "
                    << dims[0] << " != " << points.size() << std::endl;
                LBTHROW( std::runtime_error( msg.str( )));
            }
        }
        catch( const H5::Exception& ) {}

        dataset = _file.createDataSet( _d_points, pointsDT, pointsDS );
    }

    dataset.write( points.data(), H5::PredType::NATIVE_FLOAT );
}

void MorphologyHDF5::writeSections( const Vector2is& sections,
                                    const MorphologyRepairStage stage )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;

    if( _version == MORPHOLOGY_VERSION_H5_2 )
    {
        hsize_t dim[2] = { sections.size(), 2 };
        H5::DataSpace structureDS( 2, dim );

        H5::FloatType structureDT( H5::PredType::NATIVE_INT );
        structureDT.setOrder( H5T_ORDER_LE );
        H5::Group root = _file.openGroup( _g_root + "/" + _g_structure );
        H5::DataSet dataset = root.createDataSet( toString( stage ),
                                                  structureDT, structureDS );
        dataset.write( sections.data(), H5::PredType::NATIVE_INT );
        return;
    }

    H5::DataSet dataset = _getStructureDataSet( sections.size( ));
    const H5::DataSpace& dspace = dataset.getSpace();
    const hsize_t count[2] = { sections.size(), 1 };
    const hsize_t offset[2] = { 0, 1 };
    dspace.selectHyperslab( H5S_SELECT_XOR, count, offset );

    const hsize_t mdim[2] = { sections.size(), 2 };
    const H5::DataSpace mspace( 2, mdim );
    dataset.write( sections.data(), H5::PredType::NATIVE_INT, mspace,
                   dspace );
}

void MorphologyHDF5::writeSectionTypes( const SectionTypes& types )
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;

    if( _version == MORPHOLOGY_VERSION_H5_2 )
    {
        hsize_t dim[2] = { types.size(), 1 };
        H5::DataSpace structureDS( 2, dim );

        H5::FloatType structureDT( H5::PredType::NATIVE_INT );
        structureDT.setOrder( H5T_ORDER_LE );
        H5::Group root = _file.openGroup( _g_root + "/" + _g_structure );
        H5::DataSet dataset = root.createDataSet( _d_type, structureDT,
                                                  structureDS );
        dataset.write( types.data(), H5::PredType::NATIVE_INT );

        return;
    }

    H5::DataSet dataset = _getStructureDataSet( types.size( ));
    const H5::DataSpace& dspace = dataset.getSpace();
    const hsize_t count[2] = { types.size(), 1 };
    const hsize_t offset[2] = { 0, 1 };
    dspace.selectHyperslab( H5S_SELECT_SET, count, offset );

    const hsize_t mdim = types.size();
    const H5::DataSpace mspace( 1, &mdim );
    dataset.write( types.data(), H5::PredType::NATIVE_INT, mspace, dspace );
}

void MorphologyHDF5::writeApicals( const Vector2is& apicals )
{
    ASSERT_WRITE;

    if( _version != MORPHOLOGY_VERSION_H5_2 )
        LBTHROW( std::runtime_error( "Need version 2 to write apicals" ));

    if( apicals.empty( ))
        return;

    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    const H5::Group& root = _file.openGroup( _g_root );
    hsize_t dims = 2 * apicals.size();
    H5::DataSpace apicalDS( 1, &dims );
    H5::Attribute attr = root.createAttribute( _a_apical,
                                    H5::PredType::NATIVE_DOUBLE, apicalDS );
    attr.write( H5::PredType::NATIVE_INT, apicals.data( ));
}

void MorphologyHDF5::writePerimeters( const floats& perimeters )
{
    ASSERT_WRITE;

    if( _version != MORPHOLOGY_VERSION_H5_1_1 )
        LBTHROW( std::runtime_error( "Need version 1.1 to write perimeters" ));

    if( perimeters.empty( ))
    {
        if( getCellFamily() == FAMILY_GLIA )
            LBTHROW( std::runtime_error( "No empty perimeters allowed for glia "
                                         "morphology" ));
        return;
    }

    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    try
    {
        detail::SilenceHDF5 silence;
        _file.openDataSet( _d_perimeters );
        LBTHROW( std::runtime_error( "Perimeters were already written" ));
    }
    catch( const H5::Exception& ) {}

    const hsize_t dim = perimeters.size();

    try
    {
        detail::SilenceHDF5 silence;
        H5::DataSet points = _file.openDataSet( _d_points );
        hsize_t dims[2];
        points.getSpace().getSimpleExtentDims( dims );
        if( dims[0] != perimeters.size( ))
        {
            std::stringstream msg;
            msg << "Number of perimeters does not match number of points, "
                << dims[0] << " != " << perimeters.size() << std::endl;
            LBTHROW( std::runtime_error( msg.str( )));
        }
    }
    catch( const H5::Exception& ) {}

    H5::DataSpace perimeterDS( 1, &dim );

    H5::DataSet dataset = _file.createDataSet( _d_perimeters,
                                               H5::PredType::NATIVE_FLOAT,
                                               perimeterDS );
    dataset.write( perimeters.data(), H5::PredType::NATIVE_FLOAT );
}

void MorphologyHDF5::flush()
{
    lunchbox::ScopedWrite mutex( detail::_hdf5Lock );

    ASSERT_WRITE;
    _file.flush( H5F_SCOPE_GLOBAL );
}

void MorphologyHDF5::_checkVersion( const std::string& source )
{
    if( _readV11Metadata( ))
        return;

    if( _readV2Metadata( ))
        return;

    try
    {
        _resolveV1();
        _version = MORPHOLOGY_VERSION_H5_1;
        return;
    }
    catch( ... )
    {
        LBTHROW( std::runtime_error( "Unknown morphology file format for "
                                     "file " + source ));
    }
}

void MorphologyHDF5::_selectRepairStage()
{
    if( _version != MORPHOLOGY_VERSION_H5_2 )
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

void MorphologyHDF5::_resolveV1()
{
    _points = _file.openDataSet( "/" + _d_points );
    const H5::DataSpace pointsSpace = _points.getSpace();
    if( pointsSpace.getSimpleExtentNdims() != 2 ||
        pointsSpace.getSimpleExtentDims( _pointsDims ) < 0 ||
        _pointsDims[1] != _pointColumns )
    {
        LBTHROW( std::runtime_error( "Opening morphology file '" +
                      _file.getFileName() + "': bad number of dimensions in"
                      " 'points' dataspace"));
    }

    _sections = _file.openDataSet( _d_structure );
    const H5::DataSpace sectionsSpace = _sections.getSpace();
    if( sectionsSpace.getSimpleExtentNdims() != 2 ||
        sectionsSpace.getSimpleExtentDims(_sectionsDims ) < 0 ||
        _sectionsDims[1] != _structureV1Columns )
    {
        LBTHROW( std::runtime_error( "Opening morphology file '" +
                  _file.getFileName() + "': bad number of dimensions in"
                  " 'structure' dataspace"));
    }
}

void MorphologyHDF5::_writeV11Metadata( const MorphologyInitData& initData )
{
    ASSERT_WRITE;

    H5::Group metadata = _file.createGroup( _g_metadata );

    H5::EnumType familyEnum( H5::PredType::STD_U32LE );
    uint32_t enumValue = brion::FAMILY_NEURON;
    familyEnum.insert( "NEURON", &enumValue );
    enumValue = brion::FAMILY_GLIA;
    familyEnum.insert( "GLIA", &enumValue );
    familyEnum.commit( metadata, _e_family );

    const hsize_t dim = 1;
    H5::DataSpace familyDS( 1, &dim );
    H5::Attribute familyAttr = metadata.createAttribute( _a_family, familyEnum,
                                                     familyDS );
    _family = initData.getFamily();
    familyAttr.write( familyEnum, &_family );

    const std::string creator = "Brion";
    detail::addStringAttribute( metadata, _a_creator, creator );

    detail::addStringAttribute( metadata, _a_software_version,
                                Version::getString( ));

    const time_t now = ::time(0);
#ifdef _WIN32
    char* gmtString = ::ctime( &now );
#else
    char gmtString[32];
    ::ctime_r( &now, gmtString );
#endif

    std::string creation_time = gmtString;
    creation_time = creation_time.substr( 0, creation_time.size() -1 ); // ctime_r ends with \n
    detail::addStringAttribute( metadata, _a_creation_time, creation_time );

    hsize_t dims = 2;
    H5::DataSpace versionDS( 1, &dims );
    H5::Attribute versionAttr = metadata.createAttribute( _a_version,
                                      H5::PredType::STD_U32LE, versionDS );
    const uint32_t version[2] = { 1, 1 };
    versionAttr.write( H5::PredType::STD_U32LE, &version[0] );
}

bool MorphologyHDF5::_readV11Metadata()
{
    try
    {
        detail::SilenceHDF5 silence;
        const H5::Group& metadata = _file.openGroup( _g_metadata );
        const H5::Attribute& attr = metadata.openAttribute( _a_version );

        uint32_t version[2];
        attr.read( H5::PredType::STD_U32LE, &version[0] );
        if( version[0] != 1 || version[1] != 1 )
            return false;

        _version = MORPHOLOGY_VERSION_H5_1_1;

        const H5::Attribute& familyAttr = metadata.openAttribute( _a_family );
        H5::EnumType familyEnum = metadata.openEnumType( _e_family );

        familyAttr.read( familyEnum, &_family );

        _resolveV1();
        return true;
    }
    catch( const H5::Exception& )
    {
        return false;
    }
}

bool MorphologyHDF5:: _readV2Metadata()
{
    try
    {
        detail::SilenceHDF5 silence;
        const H5::Group& root = _file.openGroup( _g_root );
        const H5::Attribute& attr = root.openAttribute( _a_version );

        attr.read( H5::PredType::NATIVE_INT, &_version );

        if( _version == MORPHOLOGY_VERSION_H5_2 )
            return true;
    }
    catch( const H5::Exception& ) {}

    try
    {
        detail::SilenceHDF5 silence;
        _file.openGroup( _g_root );
        _version = MORPHOLOGY_VERSION_H5_2;
        return true;
    }
    catch( const H5::Exception& )
    {
        return false;
    }
}

void MorphologyHDF5::_writeV2Metadata()
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
    std::string creator = "Brion " + Version::getString() +
                          " brion::Morphology " + gmtString;
    creator = creator.substr( 0, creator.size() -1 ); // ctime_r ends with \n
    detail::addStringAttribute( root, _a_creator, creator );

    H5::DataSpace versionDS( H5S_SCALAR );
    H5::Attribute versionAttr = root.createAttribute( _a_version,
                                      H5::PredType::NATIVE_INT, versionDS );
    const int version = MORPHOLOGY_VERSION_H5_2;
    versionAttr.write( H5::PredType::NATIVE_INT, &version );
}

H5::DataSet MorphologyHDF5::_getStructureDataSet( size_t nSections )
{
    H5::DataSet dataset;
    try
    {
        detail::SilenceHDF5 silence;
        return _file.openDataSet( _d_structure );
    }
    catch( const H5::Exception& )
    {
        hsize_t dim[2] = { nSections, 3 };
        H5::DataSpace structureDS( 2, dim );
        H5::FloatType structureDT( H5::PredType::NATIVE_INT );
        structureDT.setOrder( H5T_ORDER_LE );
        return _file.createDataSet( _d_structure, structureDT, structureDS );
    }
}

}
}
