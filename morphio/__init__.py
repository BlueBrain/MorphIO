import platform


if platform.system() == "Windows":
    # the plan is to leverage the h5py inclusion of hdf5 dll; this way
    # there cannot be a mismatch between version, when using windows,
    # and MorphIO doesn't need to build hdf5
    import h5py as _h5py
    if (1, 14) != _h5py.version.hdf5_version_tuple[:2]:
        raise RuntimeError(f'HDF5 library version mismatch. 1.14.x != {_h5py.version.hdf5_version}')
    del _h5py


from ._morphio import (
    Annotation,
    AnnotationType,
    CellFamily,
    CellLevel,
    Collection,
    DendriticSpine,
    EndoplasmicReticulum,
    GlialCell,
    IDSequenceError,
    IterType,
    LogLevel,
    MissingParentError,
    MitoSection,
    Mitochondria,
    MitochondriaPointLevel,
    MorphioError,
    Morphology,
    MultipleTrees,
    Option,
    PointLevel,
    Points,
    PostSynapticDensity,
    Properties,
    RawDataError,
    Section,
    SectionBuilderError,
    SectionLevel,
    SectionType,
    Soma,
    SomaError,
    SomaType,
    UnknownFileType,
    VasculatureSectionType,
    Warning,
    WriterError,
    mut,
    ostream_redirect,
    set_ignored_warning,
    set_raise_warnings,
    set_maximum_warnings,
    vasculature,
    version,
)
