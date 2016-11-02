Implementing Morphology version 1.1, support for glia cells {#morphology11}
============

The current morphology version 1 specification and implementation is tailored
for neuronal cells. The support for glia does imply only some changes and will
be backward compatible to version 1, hence version 1.1.

## Requirements

* Backward-compatibility with version 1
* Support for glia cells according to
  [specification](https://bbpcode.epfl.ch/code/#/c/22473/20/source/h5v1.rst):
  * new 'cell_family' HDF5 enum which has the one of the values NEURON or GLIA
  * new glia cell values for brion::SectionType: 'glia process', 'glia endfoot'
  * new 'perimeters' dataset
* New metadata dataset containing provenance and versioning information:
  * 'creator': The software used to create the morphology.
  * 'software_version': major.minor.patch.sha1 of the software that wrote the
    file
  * 'creation_time': date & time when the file was written
  * 'version': An attribute array of two U32LE integers that stores the major
    and minor version numbers.

## New dependencies

None

## API

### Additions

    SectionTypes
    {
        // after existing ones for neurons
        SECTION_GLIA_PROCESS = 2,
        SECTION_GLIA_ENDFOOT = 3,
        // ...
    };

    MorphologyVersion
    {
        // between existing ones
        MORPHOLOGY_VERSION_H5_1 = 1,
        MORPHOLOGY_VERSION_H5_2 = 2,
        MORPHOLOGY_VERSION_H5_1_1 = 3,
        // ...
    };

    class Morphology
        // Default parameter brion::MorphologyRepairStage for all functions to
        // brion::MORPHOLOGY_UNDEFINED. Needed only for V2 and throws there if
        // brion::MORPHOLOGY_UNDEFINED.

### New

    /** The cell family represented by brion::Morphology. */
    CellFamily
    {
        FAMILY_NEURON = 0,
        FAMILY_GLIA = 1
    };

    class Morphology
    {
        ...
        /**
         * Open the given morphology file for write access.
         *
         * @param file filename of the output morphology file
         * @param family the cell family that this morphology represents.
         */
        Morphology( const std::string& file, CellFamily family );

        /** @return the cell family of that morphology. */
        CellFamily brion::getCellFamily() const;

        /**
         * @return perimeters of the cross sections for each point of the
         *         morphology in micrometers.
         * @throw std::runtime_error if empty for FAMILY_GLIA
         */
        floatsPtr readPerimeters() const;

        /**
         * Write perimeters of morphology.
         *
         * @param perimeters perimeters of the cross sections for each point of
         *                   the morphology in micrometers
         * @throw std::runtime_error if object not created with write ctor
         * @throw std::runtime_error if not supported by implementation
         */
        void writePerimeters( const floats& perimeters );

## File format

* new root dataset 'perimeters' with dimension 1 and datatype H5::NATIVE_FLOAT
* new root group 'metadata'
* new enum 'cell_family_enum' with datatype H5::NATIVE_INT and values
  NEURON=0 and GLIA=1 inside 'metadata'
* new attribute 'cell_family' with datatype 'cell_family_enum' inside 'metadata'
* new attributes 'creator', 'software_version', 'creation_time' with datatype
  H5::C_S1 (string) inside 'metadata'
* new attribute 'version' with datatype H5::STD_U32LE and dimensionality 2
  inside 'metadata'

## Examples

    brion::Morphology glia( "glia.h5", brion::FAMILY_GLIA );
    glia.writePoints( points );
    glia.writeSections( sections );
    glia.writeSectionTypes( types );
    glia.writePerimeters( perimeters );

    const brion::Morphology gliaRead( "glia.h5" );
    BOOST_CHECK_EQUAL( gliaRead.getCellFamily(), brion::FAMILY_GLIA );

    brion::Morphology neuron( "neuron.h5", brion::FAMILY_NEURON );
    neuron.writePoints( points );
    neuron.writeSections( sections );
    neuron.writeSectionTypes( types );
    neuron.writePerimeters( perimeters );

    const brion::Morphology neuronRead( "neuron.h5" );
    BOOST_CHECK_EQUAL( neuronRead.getCellFamily(), brion::FAMILY_NEURON );

## Issues

### 1: Do we choose cell family automatically or do we require an explicit selection?

_Resolved: Explicit selection

As the perimeters can also be optionally used for the neuron cell family, a new
constructor for writing morphologies enforces to explicitly choose the cell
family.

### 2. Why is the metadata not exposed in the API?

_Resolved: No use case at the moment

From current use cases it is not needed to query the metadata programmatically.
It will be used internally to verify validity of files while loading them. The
user can check the metadata values by using _h5dump_.
