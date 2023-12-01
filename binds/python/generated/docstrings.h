/*
  This file contains docstrings for use in the Python bindings.
  Do not edit! They were automatically extracted by pybind11_mkdoc.
 */

#define MKD_EXPAND(x)                                      x
#define MKD_COUNT(_1, _2, _3, _4, _5, _6, _7, COUNT, ...)  COUNT
#define MKD_VA_SIZE(...)                                   MKD_EXPAND(MKD_COUNT(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0))
#define MKD_CAT1(a, b)                                     a ## b
#define MKD_CAT2(a, b)                                     MKD_CAT1(a, b)
#define MKD_DOC1(n1)                                       mkd_doc_##n1
#define MKD_DOC2(n1, n2)                                   mkd_doc_##n1##_##n2
#define MKD_DOC3(n1, n2, n3)                               mkd_doc_##n1##_##n2##_##n3
#define MKD_DOC4(n1, n2, n3, n4)                           mkd_doc_##n1##_##n2##_##n3##_##n4
#define MKD_DOC5(n1, n2, n3, n4, n5)                       mkd_doc_##n1##_##n2##_##n3##_##n4##_##n5
#define MKD_DOC7(n1, n2, n3, n4, n5, n6, n7)               mkd_doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6##_##n7
#define DOC(...)                                           MKD_EXPAND(MKD_EXPAND(MKD_CAT2(MKD_DOC, MKD_VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif


static const char *mkd_doc_morphio_Collection = R"doc()doc";

static const char *mkd_doc_morphio_CollectionImpl = R"doc()doc";

static const char *mkd_doc_morphio_Collection_Collection = R"doc()doc";

static const char *mkd_doc_morphio_Collection_Collection_2 =
R"doc(Create a collection from the given path.

If `collection_path` points to an HDF5 file, then that file must be a
container. Otherwise the `collection_path` should point to the
directory containing the morphology files.

If the collection path is a directory, the extension of the morphology
file must be guessed. The optional argument `extensions` specifies
which and in which order the morphologies are searched.)doc";

static const char *mkd_doc_morphio_Collection_argsort =
R"doc(Returns the reordered loop indices.

This is the suggested order in which one should load the morphologies
to minimize seeking within the file.

Note: This API is 'experimental', meaning it might change in the
future.)doc";

static const char *mkd_doc_morphio_Collection_close =
R"doc(Close the collection.

Note that `morphio::Collection` uses RAII. Therefore, the usual
scoping rules should suffice. However, there are cases where one wants
to close the collection explicitly, causing all resources held by the
collection to be release. In the case of containers it ensures that
the file is closed.

The object should not be used after calling `close`.)doc";

static const char *mkd_doc_morphio_Collection_collection = R"doc()doc";

static const char *mkd_doc_morphio_Collection_load = R"doc(Load the morphology as an immutable morphology.)doc";

static const char *mkd_doc_morphio_Collection_load_2 = R"doc(Load the morphology as a mutable morphology.)doc";

static const char *mkd_doc_morphio_Collection_load_unordered =
R"doc(Returns an iterable of loop index, morphology pairs.

See `LoadUnordered` for details.)doc";

static const char *mkd_doc_morphio_DendriticSpine = R"doc(Class to represent morphologies of dendritic spines)doc";

static const char *mkd_doc_morphio_DendriticSpine_2 = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_DendriticSpine = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_DendriticSpine_2 = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_annotations = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_endoplasmicReticulum = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_markers = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_mitochondria = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_postSynapticDensity = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_soma = R"doc()doc";

static const char *mkd_doc_morphio_DendriticSpine_somaType = R"doc()doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum =
R"doc(The entry-point class to access endoplasmic reticulum data

Spec https://bbpteam.epfl.ch/documentation/projects/Morphology%20Docum
entation/latest/h5v1.html)doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_2 = R"doc()doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_EndoplasmicReticulum = R"doc()doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_filamentCounts = R"doc(Returns the number of filaments for each neuronal section)doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_properties = R"doc()doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_sectionIndices = R"doc(Returns the list of neuronal section indices)doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_surfaceAreas = R"doc(Returns the surface areas for each neuronal section)doc";

static const char *mkd_doc_morphio_EndoplasmicReticulum_volumes = R"doc(Returns the volumes for each neuronal section)doc";

static const char *mkd_doc_morphio_GlialCell = R"doc(Class to represent morphologies of glial cells)doc";

static const char *mkd_doc_morphio_GlialCell_GlialCell = R"doc()doc";

static const char *mkd_doc_morphio_GlialCell_soma = R"doc()doc";

static const char *mkd_doc_morphio_IDSequenceError = R"doc()doc";

static const char *mkd_doc_morphio_IDSequenceError_IDSequenceError = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered =
R"doc(An iterable of loop index and morphologies.

When reading from containers, the order in which morphologies are read
can have a large impact on the overall time to load those
morphologies.

This iterator provides means of reordering loops to optimize the
access pattern. Loops such as the following

for(size_t k = 0; k < morphology_names.size; ++k) { auto morph =
collection.load<M>(morphology_names[k]); f(k, morph); }

can be replaced with

for(auto [k, morph] : collection.load_unordered<M>(morphology_names))
{ assert(collection.load<M>(morphology_names[k]) == morph); f(k,
morph); }

The order in which the morphologies are returned in unspecified, but
the loop index `k` can be used to retrieve the correct state
corresponding to iteration `k` of the original loop.

Note, that it is safe for an `LoadUnordered` object to outlive its
`collection`. Internally a shallow copy of the original `collection`
is stored inside of and kept alive for the life time of the
`LoadUnordered` object.

Note: This API is 'experimental', meaning it might change in the
future.)doc";

static const char *mkd_doc_morphio_LoadUnordered_2 =
R"doc(An iterable of loop index and morphologies.

When reading from containers, the order in which morphologies are read
can have a large impact on the overall time to load those
morphologies.

This iterator provides means of reordering loops to optimize the
access pattern. Loops such as the following

for(size_t k = 0; k < morphology_names.size; ++k) { auto morph =
collection.load<M>(morphology_names[k]); f(k, morph); }

can be replaced with

for(auto [k, morph] : collection.load_unordered<M>(morphology_names))
{ assert(collection.load<M>(morphology_names[k]) == morph); f(k,
morph); }

The order in which the morphologies are returned in unspecified, but
the loop index `k` can be used to retrieve the correct state
corresponding to iteration `k` of the original loop.

Note, that it is safe for an `LoadUnordered` object to outlive its
`collection`. Internally a shallow copy of the original `collection`
is stored inside of and kept alive for the life time of the
`LoadUnordered` object.

Note: This API is 'experimental', meaning it might change in the
future.)doc";

static const char *mkd_doc_morphio_LoadUnordered_3 = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_4 = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnorderedImpl = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_Iterator = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_k = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_load_unordered_impl = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_operator_inc = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_operator_inc_2 = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_operator_mul = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_operator_mul_2 = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_Iterator_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_LoadUnordered = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_begin = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_end = R"doc()doc";

static const char *mkd_doc_morphio_LoadUnordered_load_unordered_impl = R"doc()doc";

static const char *mkd_doc_morphio_MissingParentError = R"doc()doc";

static const char *mkd_doc_morphio_MissingParentError_MissingParentError = R"doc()doc";

static const char *mkd_doc_morphio_MitoSection = R"doc(Mitochondria section)doc";

static const char *mkd_doc_morphio_MitoSection_2 = R"doc()doc";

static const char *mkd_doc_morphio_MitoSection_MitoSection = R"doc()doc";

static const char *mkd_doc_morphio_MitoSection_breadth_begin = R"doc(Breadth first search iterator)doc";

static const char *mkd_doc_morphio_MitoSection_breadth_end = R"doc()doc";

static const char *mkd_doc_morphio_MitoSection_depth_begin = R"doc(Depth first search iterator)doc";

static const char *mkd_doc_morphio_MitoSection_depth_end = R"doc()doc";

static const char *mkd_doc_morphio_MitoSection_diameters = R"doc(Returns list of section's point diameters)doc";

static const char *mkd_doc_morphio_MitoSection_hasSameShape =
R"doc(Return true if the both sections have the same neuriteSectionIds,
diameters and relativePathLengths)doc";

static const char *mkd_doc_morphio_MitoSection_neuriteSectionIds = R"doc(Returns list of neuronal section IDs associated to each point)doc";

static const char *mkd_doc_morphio_MitoSection_relativePathLengths =
R"doc(Returns list of relative distances between the start of the neuronal
section and each point of the mitochondrial section\n Note: - a
relative distance of 0 means the mitochondrial point is at the
beginning of the neuronal section - a relative distance of 1 means the
mitochondrial point is at the end of the neuronal section)doc";

static const char *mkd_doc_morphio_MitoSection_upstream_begin = R"doc(Upstream first search iterator)doc";

static const char *mkd_doc_morphio_MitoSection_upstream_end = R"doc()doc";

static const char *mkd_doc_morphio_Mitochondria =
R"doc(The entry-point class to access mitochondrial data

By design, it is the equivalent of the Morphology class but at the
mitochondrial level. As the Morphology class, it implements a section
accessor and a root section accessor returning views on the Properties
object for the queried mitochondrial section.)doc";

static const char *mkd_doc_morphio_Mitochondria_2 = R"doc()doc";

static const char *mkd_doc_morphio_Mitochondria_Mitochondria = R"doc()doc";

static const char *mkd_doc_morphio_Mitochondria_properties = R"doc()doc";

static const char *mkd_doc_morphio_Mitochondria_rootSections = R"doc(Return a vector of all root sections)doc";

static const char *mkd_doc_morphio_Mitochondria_section = R"doc(Return the Section with the given id.)doc";

static const char *mkd_doc_morphio_Mitochondria_sections =
R"doc(Return a vector containing all section objects

Notes: Soma is not included)doc";

static const char *mkd_doc_morphio_MorphioError = R"doc(Base class of all morphio errors)doc";

static const char *mkd_doc_morphio_MorphioError_MorphioError = R"doc()doc";

static const char *mkd_doc_morphio_Morphology =
R"doc(Class that gives read access to a Morphology file.

Following RAII, this class is ready to use after the creation and will
ensure release of resources upon destruction.)doc";

static const char *mkd_doc_morphio_Morphology_2 = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_Morphology = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_Morphology_2 = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_Morphology_3 =
R"doc(Open the given source to a morphology file and parse it.

Parameter ``source``:
    path to a source file.

Parameter ``options``:
    is the modifier flags to be applied. All flags are defined in
    their corresponding morphio.enums.Option and can be composed.

Example: Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);)doc";

static const char *mkd_doc_morphio_Morphology_Morphology_4 = R"doc(Constructor from an already parsed file)doc";

static const char *mkd_doc_morphio_Morphology_Morphology_5 = R"doc(Constructor from an instance of morphio::mut::Morphology)doc";

static const char *mkd_doc_morphio_Morphology_Morphology_6 = R"doc(Load a morphology from a string)doc";

static const char *mkd_doc_morphio_Morphology_Morphology_7 = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_annotations = R"doc(Return the annotation object)doc";

static const char *mkd_doc_morphio_Morphology_breadth_begin =
R"doc(Breadth first iterator

If id == -1, the iteration will be successively performed starting at
each root section)doc";

static const char *mkd_doc_morphio_Morphology_breadth_end = R"doc(breadth end iterator)doc";

static const char *mkd_doc_morphio_Morphology_cellFamily = R"doc(Return the cell family (neuron or glia))doc";

static const char *mkd_doc_morphio_Morphology_connectivity =
R"doc(Return the graph connectivity of the morphology where each section is
seen as a node Note: -1 is the soma node)doc";

static const char *mkd_doc_morphio_Morphology_depth_begin =
R"doc(Depth first iterator starting at a given section id

If id == -1, the iteration will start at each root section,
successively)doc";

static const char *mkd_doc_morphio_Morphology_depth_end = R"doc(depth end iterator)doc";

static const char *mkd_doc_morphio_Morphology_diameters =
R"doc(Return a vector with all diameters from all sections (soma points are
not included))doc";

static const char *mkd_doc_morphio_Morphology_endoplasmicReticulum = R"doc(Return the endoplasmic reticulum object)doc";

static const char *mkd_doc_morphio_Morphology_get = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_markers = R"doc(Return the markers)doc";

static const char *mkd_doc_morphio_Morphology_mitochondria = R"doc(Return the mitochondria object)doc";

static const char *mkd_doc_morphio_Morphology_operator_assign = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_operator_assign_2 = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_perimeters = R"doc(Return a vector with all perimeters from all sections)doc";

static const char *mkd_doc_morphio_Morphology_points =
R"doc(Return a vector with all points from all sections (soma points are not
included))doc";

static const char *mkd_doc_morphio_Morphology_properties = R"doc()doc";

static const char *mkd_doc_morphio_Morphology_rootSections = R"doc(Return a vector of all root sections (sections whose parent ID are -1))doc";

static const char *mkd_doc_morphio_Morphology_section =
R"doc(Return the Section with the given id.

Throws:
    RawDataError if the id is out of range)doc";

static const char *mkd_doc_morphio_Morphology_sectionOffsets =
R"doc(Returns a list with offsets to access data of a specific section in
the points and diameters arrays.

Example: accessing diameters of n'th section will be located in the
Morphology::diameters array from diameters[sectionOffsets(n)] to
diameters[sectionOffsets(n+1)-1]

Note: for convenience, the last point of this array is the points()
array size so that the above example works also for the last section.)doc";

static const char *mkd_doc_morphio_Morphology_sectionTypes = R"doc(Return a vector with the section type of every section)doc";

static const char *mkd_doc_morphio_Morphology_sections =
R"doc(Return a vector containing all section objects

Notes: Soma is not included)doc";

static const char *mkd_doc_morphio_Morphology_soma = R"doc(Return the soma object)doc";

static const char *mkd_doc_morphio_Morphology_somaType = R"doc(Return the soma type)doc";

static const char *mkd_doc_morphio_Morphology_version = R"doc(Return the version)doc";

static const char *mkd_doc_morphio_MultipleTrees = R"doc()doc";

static const char *mkd_doc_morphio_MultipleTrees_MultipleTrees = R"doc()doc";

static const char *mkd_doc_morphio_NotImplementedError = R"doc()doc";

static const char *mkd_doc_morphio_NotImplementedError_NotImplementedError = R"doc()doc";

static const char *mkd_doc_morphio_Property_Annotation = R"doc(Class that holds service information about a warning.)doc";

static const char *mkd_doc_morphio_Property_Annotation_Annotation = R"doc()doc";

static const char *mkd_doc_morphio_Property_Annotation_details = R"doc()doc";

static const char *mkd_doc_morphio_Property_Annotation_lineNumber = R"doc()doc";

static const char *mkd_doc_morphio_Property_Annotation_points = R"doc()doc";

static const char *mkd_doc_morphio_Property_Annotation_sectionId = R"doc()doc";

static const char *mkd_doc_morphio_Property_Annotation_type = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel =
R"doc(Service information that is available at the Morphology level
(morphology version, morphology family, soma type, etc.))doc";

static const char *mkd_doc_morphio_Property_CellLevel_annotations = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_cellFamily = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_diff = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_fileFormat = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_majorVersion = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_markers = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_minorVersion = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_somaType = R"doc()doc";

static const char *mkd_doc_morphio_Property_CellLevel_version = R"doc()doc";

static const char *mkd_doc_morphio_Property_DendriticSpine_Level = R"doc()doc";

static const char *mkd_doc_morphio_Property_DendriticSpine_Level_post_synaptic_density = R"doc()doc";

static const char *mkd_doc_morphio_Property_DendriticSpine_PostSynapticDensity = R"doc()doc";

static const char *mkd_doc_morphio_Property_DendriticSpine_PostSynapticDensity_offset = R"doc()doc";

static const char *mkd_doc_morphio_Property_DendriticSpine_PostSynapticDensity_sectionId = R"doc()doc";

static const char *mkd_doc_morphio_Property_DendriticSpine_PostSynapticDensity_segmentId = R"doc()doc";

static const char *mkd_doc_morphio_Property_Diameter = R"doc()doc";

static const char *mkd_doc_morphio_Property_EndoplasmicReticulumLevel =
R"doc(Information that is available at the endoplasmic reticulum end level
(section indices, volumes, surface areas, filament counts))doc";

static const char *mkd_doc_morphio_Property_EndoplasmicReticulumLevel_filamentCounts = R"doc()doc";

static const char *mkd_doc_morphio_Property_EndoplasmicReticulumLevel_sectionIndices = R"doc()doc";

static const char *mkd_doc_morphio_Property_EndoplasmicReticulumLevel_surfaceAreas = R"doc()doc";

static const char *mkd_doc_morphio_Property_EndoplasmicReticulumLevel_volumes = R"doc()doc";

static const char *mkd_doc_morphio_Property_Marker =
R"doc(A marker within the morphology. It is used in ASC only and stores some
additional info about a morphology point.)doc";

static const char *mkd_doc_morphio_Property_Marker_label = R"doc()doc";

static const char *mkd_doc_morphio_Property_Marker_pointLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Marker_sectionId = R"doc(id of section that contains the marker)doc";

static const char *mkd_doc_morphio_Property_MitoDiameter = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitoNeuriteSectionId = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitoPathLength = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitoSection = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel =
R"doc(Information that is available at the mitochondrial point level
(enclosing neuronal section, relative distance to start of neuronal
section, diameter))doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_MitochondriaPointLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_MitochondriaPointLevel_2 = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_MitochondriaPointLevel_3 = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_diameters = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_diff = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_relativePathLengths = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaPointLevel_sectionIds = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaSectionLevel =
R"doc(Information that is available at the mitochondrial section level
(parent section))doc";

static const char *mkd_doc_morphio_Property_MitochondriaSectionLevel_children = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaSectionLevel_diff = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaSectionLevel_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaSectionLevel_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_Property_MitochondriaSectionLevel_sections = R"doc()doc";

static const char *mkd_doc_morphio_Property_Perimeter = R"doc()doc";

static const char *mkd_doc_morphio_Property_Point = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel =
R"doc(Information that is available at the point level (point coordinate,
diameter, perimeter))doc";

static const char *mkd_doc_morphio_Property_PointLevel_PointLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_PointLevel_2 = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_PointLevel_3 = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_PointLevel_4 = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_diameters = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_operator_assign = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_perimeters = R"doc()doc";

static const char *mkd_doc_morphio_Property_PointLevel_points = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties = R"doc(The lowest level data blob)doc";

static const char *mkd_doc_morphio_Property_Properties_2 = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_cellFamily = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_cellLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_children = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_dendriticSpineLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_endoplasmicReticulumLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_get = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_get_mut = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_mitochondriaPointLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_mitochondriaSectionLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_pointLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_sectionLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_somaLevel = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_somaType = R"doc()doc";

static const char *mkd_doc_morphio_Property_Properties_version = R"doc()doc";

static const char *mkd_doc_morphio_Property_Section = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionLevel =
R"doc(Information that is available at the section level (section type,
parent section))doc";

static const char *mkd_doc_morphio_Property_SectionLevel_children = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionLevel_diff = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionLevel_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionLevel_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionLevel_sectionTypes = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionLevel_sections = R"doc()doc";

static const char *mkd_doc_morphio_Property_SectionType = R"doc()doc";

static const char *mkd_doc_morphio_Property_children = R"doc()doc";

static const char *mkd_doc_morphio_Property_children_2 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_2 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_3 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_4 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_5 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_6 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_7 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_8 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_9 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_2 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_3 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_4 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_5 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_6 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_7 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_8 = R"doc()doc";

static const char *mkd_doc_morphio_Property_get_mut_9 = R"doc()doc";

static const char *mkd_doc_morphio_Property_operator_lshift = R"doc()doc";

static const char *mkd_doc_morphio_Property_operator_lshift_2 = R"doc()doc";

static const char *mkd_doc_morphio_RawDataError = R"doc()doc";

static const char *mkd_doc_morphio_RawDataError_RawDataError = R"doc()doc";

static const char *mkd_doc_morphio_Section =
R"doc(A class to represent a morphological section.

A Section is an unbranched piece of a morphological skeleton. This
class provides functions to query information about the sample points
that compose the section and functions to obtain the parent and
children sections.

The cell soma is also considered a section, but some functions have
special meaning for it.

Sections cannot be directly created, but are returned by several
morphio::Morphology and morphio::Section methods.

This is a lightweight object with STL container style thread safety.
It is also safe to use a section after the morphology from where it
comes has been deallocated. The morphological data will be kept as
long as there is a Section referring to it.)doc";

static const char *mkd_doc_morphio_Section_2 = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase =
R"doc(This CRTP
(https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
defines basic methods that every kind of sections (should them be
neuronal or mithochondrial) must define.

The CRTP is used here so that the methods of the base class can return
object of the derived class. Examples: T SectionBase::parent()
std::vector<T> SectionBase::children())doc";

static const char *mkd_doc_morphio_SectionBase_2 = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_3 = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_SectionBase = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_children = R"doc(Return a list of children sections)doc";

static const char *mkd_doc_morphio_SectionBase_get = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_id = R"doc(Return the ID of this section.)doc";

static const char *mkd_doc_morphio_SectionBase_id_2 = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_isRoot = R"doc(Return true if this section is a root section (parent ID == -1))doc";

static const char *mkd_doc_morphio_SectionBase_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_parent =
R"doc(Return the parent section of this section

Throws:
    MissingParentError is the section doesn't have a parent.)doc";

static const char *mkd_doc_morphio_SectionBase_properties = R"doc()doc";

static const char *mkd_doc_morphio_SectionBase_range = R"doc()doc";

static const char *mkd_doc_morphio_SectionBuilderError = R"doc()doc";

static const char *mkd_doc_morphio_SectionBuilderError_SectionBuilderError = R"doc()doc";

static const char *mkd_doc_morphio_Section_Section = R"doc()doc";

static const char *mkd_doc_morphio_Section_breadth_begin = R"doc(Breadth first iterator)doc";

static const char *mkd_doc_morphio_Section_breadth_end = R"doc()doc";

static const char *mkd_doc_morphio_Section_depth_begin = R"doc(Depth first iterator)doc";

static const char *mkd_doc_morphio_Section_depth_end = R"doc()doc";

static const char *mkd_doc_morphio_Section_diameters =
R"doc(Return a view
(https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-
intro.md#gslspan-what-is-gslspan-and-what-is-it-for) to this section's
point diameters)doc";

static const char *mkd_doc_morphio_Section_hasSameShape =
R"doc(Return true if the both sections have the same points, diameters and
perimeters)doc";

static const char *mkd_doc_morphio_Section_isHeterogeneous =
R"doc(Return true if the sections of the tree downstream (downstream = true)
or upstream (donwstream = false) have the same section type as the
current section.)doc";

static const char *mkd_doc_morphio_Section_perimeters =
R"doc(Return a view
(https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-
intro.md#gslspan-what-is-gslspan-and-what-is-it-for) to this section's
point perimeters)doc";

static const char *mkd_doc_morphio_Section_points =
R"doc(Return a view
(https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-
intro.md#gslspan-what-is-gslspan-and-what-is-it-for) to this section's
point coordinates)doc";

static const char *mkd_doc_morphio_Section_type = R"doc(Return the morphological type of this section (dendrite, axon, ...))doc";

static const char *mkd_doc_morphio_Section_upstream_begin = R"doc(Upstream iterator)doc";

static const char *mkd_doc_morphio_Section_upstream_end = R"doc()doc";

static const char *mkd_doc_morphio_Soma =
R"doc(A class to represent a neuron soma.

This class provides functions to query information about the soma of a
neuron.

Typically the soma is described as the poly-line of the projection of
the soma onto a plane, where the plane normal points in the vertical
direction in the local coordinate system of the morphology. In other
cases the poly-line is not projected onto a plane, but is an
approximation of the countour of the soma as seen in an orhogonal
projection down the vertical axis (this is basically the same as
before, but the vertical coordinate is not 0 for all the points). This
class can also be used for both descriptions as well as somas simply
approximated as spheres.

The coordinates system used by a soma will be in the same as the
brain::Morphology from where it comes.

@version unstable)doc";

static const char *mkd_doc_morphio_Soma_2 = R"doc()doc";

static const char *mkd_doc_morphio_SomaError = R"doc()doc";

static const char *mkd_doc_morphio_SomaError_SomaError = R"doc()doc";

static const char *mkd_doc_morphio_Soma_Soma = R"doc()doc";

static const char *mkd_doc_morphio_Soma_center = R"doc(Return the center of gravity of the soma points)doc";

static const char *mkd_doc_morphio_Soma_diameters = R"doc(Return the diameters of all soma points)doc";

static const char *mkd_doc_morphio_Soma_maxDistance =
R"doc(Return the maximum distance between the center of gravity and any of
the soma points)doc";

static const char *mkd_doc_morphio_Soma_points = R"doc(Return the coordinates (x,y,z) of all soma points)doc";

static const char *mkd_doc_morphio_Soma_properties = R"doc()doc";

static const char *mkd_doc_morphio_Soma_surface =
R"doc(Return the soma surface\n" Note: the soma surface computation depends
on the soma type)doc";

static const char *mkd_doc_morphio_Soma_type = R"doc(Return the soma type)doc";

static const char *mkd_doc_morphio_Soma_volume =
R"doc(Return the soma volume\n" Note: the soma volume computation depends on
the soma type)doc";

static const char *mkd_doc_morphio_UnknownFileType = R"doc()doc";

static const char *mkd_doc_morphio_UnknownFileType_UnknownFileType = R"doc()doc";

static const char *mkd_doc_morphio_WriterError = R"doc()doc";

static const char *mkd_doc_morphio_WriterError_WriterError = R"doc()doc";

static const char *mkd_doc_morphio_children = R"doc(Return a list of children sections)doc";

static const char *mkd_doc_morphio_diff = R"doc(Perform a diff on 2 morphologies, returns True if items differ)doc";

static const char *mkd_doc_morphio_diff_2 = R"doc(Perform a diff on 2 sections, returns True if items differ)doc";

static const char *mkd_doc_morphio_enable_if_immutable = R"doc(Enable if `T` is a immutable morphology.)doc";

static const char *mkd_doc_morphio_enable_if_mutable = R"doc(Enable if `T` is a mutable morphology.)doc";

static const char *mkd_doc_morphio_enums_AnnotationType = R"doc()doc";

static const char *mkd_doc_morphio_enums_AnnotationType_SINGLE_CHILD = R"doc()doc";

static const char *mkd_doc_morphio_enums_CellFamily = R"doc(The cell family represented by morphio::Morphology.)doc";

static const char *mkd_doc_morphio_enums_CellFamily_GLIA = R"doc(Glia)doc";

static const char *mkd_doc_morphio_enums_CellFamily_NEURON = R"doc(Neuron)doc";

static const char *mkd_doc_morphio_enums_CellFamily_SPINE = R"doc(Spine)doc";

static const char *mkd_doc_morphio_enums_LogLevel = R"doc()doc";

static const char *mkd_doc_morphio_enums_LogLevel_DEBUG = R"doc()doc";

static const char *mkd_doc_morphio_enums_LogLevel_ERROR = R"doc()doc";

static const char *mkd_doc_morphio_enums_LogLevel_INFO = R"doc()doc";

static const char *mkd_doc_morphio_enums_LogLevel_WARNING = R"doc()doc";

static const char *mkd_doc_morphio_enums_Option =
R"doc(The list of modifier flags that can be passed when loading a
morphology See morphio::mut::modifiers for more information)doc";

static const char *mkd_doc_morphio_enums_Option_NO_DUPLICATES = R"doc(Skip duplicating points)doc";

static const char *mkd_doc_morphio_enums_Option_NO_MODIFIER = R"doc(Read morphology as is without any modification)doc";

static const char *mkd_doc_morphio_enums_Option_NRN_ORDER = R"doc(Order of neurites will be the same as in NEURON simulator)doc";

static const char *mkd_doc_morphio_enums_Option_SOMA_SPHERE = R"doc(Interpret morphology soma as a sphere)doc";

static const char *mkd_doc_morphio_enums_Option_TWO_POINTS_SECTIONS = R"doc(Read sections only with 2 or more points)doc";

static const char *mkd_doc_morphio_enums_SectionType = R"doc(Classification of neuron substructures.)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_ALL = R"doc(Any section type)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_APICAL_DENDRITE = R"doc(Apical dendrite (far from soma))doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_AXON = R"doc(Axon section)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CNIC_CUSTOM_START = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_10 = R"doc(Custom section type 10)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_11 = R"doc(Custom section type 11)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_12 = R"doc(Custom section type 12)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_13 = R"doc(Custom section type 13)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_14 = R"doc(Custom section type 14)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_15 = R"doc(Custom section type 15)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_16 = R"doc(Custom section type 16)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_17 = R"doc(Custom section type 17)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_18 = R"doc(Custom section type 18)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_19 = R"doc(Custom section type 19)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_5 = R"doc(Custom section type 5)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_6 = R"doc(Custom section type 6)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_7 = R"doc(Custom section type 7)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_8 = R"doc(Custom section type 8)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_CUSTOM_9 = R"doc(Custom section type 9)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_DENDRITE = R"doc(General or basal dendrite (near to soma))doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_GLIA_PERIVASCULAR_PROCESS = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_GLIA_PROCESS = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_OUT_OF_RANGE_START = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_SOMA = R"doc(Neuron cell body)doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_SPINE_HEAD = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_SPINE_NECK = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SECTION_UNDEFINED = R"doc(Undefined section)doc";

static const char *mkd_doc_morphio_enums_SectionType_SWC_SECTION_END_POINT = R"doc()doc";

static const char *mkd_doc_morphio_enums_SectionType_SWC_SECTION_FORK_POINT = R"doc()doc";

static const char *mkd_doc_morphio_enums_SomaType = R"doc(Soma type.)doc";

static const char *mkd_doc_morphio_enums_SomaType_SOMA_CYLINDERS = R"doc(Soma made of cylinders)doc";

static const char *mkd_doc_morphio_enums_SomaType_SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS = R"doc(Soma made of three cylinders)doc";

static const char *mkd_doc_morphio_enums_SomaType_SOMA_SIMPLE_CONTOUR = R"doc(Contour soma. Assumed that contour is in XY plane)doc";

static const char *mkd_doc_morphio_enums_SomaType_SOMA_SINGLE_POINT = R"doc(Single point soma)doc";

static const char *mkd_doc_morphio_enums_SomaType_SOMA_UNDEFINED = R"doc(Undefined soma)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType = R"doc(Classification of vasculature section types.)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_ARTERIAL_CAPILLARY = R"doc(Arterial capillary)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_ARTERIOLE = R"doc(Arteriole)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_ARTERY = R"doc(Artery)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_CUSTOM = R"doc(Custom section type)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_NOT_DEFINED = R"doc(Undefined)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_TRANSITIONAL = R"doc(Transitional)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_VEIN = R"doc(Vein)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_VENOUS_CAPILLARY = R"doc(Venous capillary)doc";

static const char *mkd_doc_morphio_enums_VascularSectionType_SECTION_VENULE = R"doc(Venule)doc";

static const char *mkd_doc_morphio_enums_Warning =
R"doc(All possible warnings that can be ignored by user. This enum should be
kept in sync with the warnings defined in morphio::ErrorMessages and
with binds/python/bind_misc.cpp)doc";

static const char *mkd_doc_morphio_enums_Warning_APPENDING_EMPTY_SECTION = R"doc(Appending of an empty section)doc";

static const char *mkd_doc_morphio_enums_Warning_DISCONNECTED_NEURITE = R"doc(Found a disconnected neurite in a morphology)doc";

static const char *mkd_doc_morphio_enums_Warning_MITOCHONDRIA_WRITE_NOT_SUPPORTED = R"doc(Mitochondria can be saved in H5 format only)doc";

static const char *mkd_doc_morphio_enums_Warning_NO_SOMA_FOUND = R"doc(No soma found in a file)doc";

static const char *mkd_doc_morphio_enums_Warning_ONLY_CHILD = R"doc(Single child sections are not allowed in SWC format)doc";

static const char *mkd_doc_morphio_enums_Warning_SOMA_NON_CONFORM = R"doc(Soma does not conform the three point soma spec from NeuroMorpho.org)doc";

static const char *mkd_doc_morphio_enums_Warning_SOMA_NON_CONTOUR = R"doc(Soma must be a contour for ASC and H5)doc";

static const char *mkd_doc_morphio_enums_Warning_SOMA_NON_CYLINDER_OR_POINT = R"doc(Soma must be stacked cylinders or a point)doc";

static const char *mkd_doc_morphio_enums_Warning_UNDEFINED = R"doc(undefined value)doc";

static const char *mkd_doc_morphio_enums_Warning_WRITE_EMPTY_MORPHOLOGY = R"doc(Writing empty morphology)doc";

static const char *mkd_doc_morphio_enums_Warning_WRITE_NO_SOMA = R"doc(Writing without a soma)doc";

static const char *mkd_doc_morphio_enums_Warning_WRITE_UNDEFINED_SOMA = R"doc(Soma is UNDEFINED)doc";

static const char *mkd_doc_morphio_enums_Warning_WRONG_DUPLICATE = R"doc(A wrong duplicate point in a section)doc";

static const char *mkd_doc_morphio_enums_Warning_WRONG_ROOT_POINT = R"doc(A wrong root point of a neurite in the case of 3 points soma)doc";

static const char *mkd_doc_morphio_enums_Warning_ZERO_DIAMETER = R"doc(Zero section diameter)doc";

static const char *mkd_doc_morphio_enums_operator_lshift = R"doc()doc";

static const char *mkd_doc_morphio_get = R"doc()doc";

static const char *mkd_doc_morphio_getVersionString = R"doc()doc";

static const char *mkd_doc_morphio_isRoot = R"doc(Return true if this section is a root section (parent ID == -1))doc";

static const char *mkd_doc_morphio_mut_DendriticSpine = R"doc(Mutable(editable) morphio::DendriticSpine)doc";

static const char *mkd_doc_morphio_mut_DendriticSpine_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_DendriticSpine_DendriticSpine = R"doc()doc";

static const char *mkd_doc_morphio_mut_DendriticSpine_DendriticSpine_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_DendriticSpine_postSynapticDensity = R"doc(Returns the post synaptic density values)doc";

static const char *mkd_doc_morphio_mut_DendriticSpine_postSynapticDensity_2 = R"doc(Returns the post synaptic density values)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum = R"doc(Mutable(editable) morphio::EndoplasmicReticulum)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_EndoplasmicReticulum = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_EndoplasmicReticulum_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_EndoplasmicReticulum_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_EndoplasmicReticulum_4 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_EndoplasmicReticulum_5 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_buildReadOnly =
R"doc(Returns the data structure that stores ER data This data structure is
used to create the immutable object)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_filamentCounts = R"doc(Returns the number of filaments for each neuronal section)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_filamentCounts_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_properties = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_sectionIndices = R"doc(Returns the list of neuronal section indices)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_sectionIndices_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_surfaceAreas = R"doc(Returns the surface areas for each neuronal section)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_surfaceAreas_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_volumes = R"doc(Returns the volumes for each neuronal section)doc";

static const char *mkd_doc_morphio_mut_EndoplasmicReticulum_volumes_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_GlialCell = R"doc(Mutable(editable) morphio::GlialCell)doc";

static const char *mkd_doc_morphio_mut_GlialCell_GlialCell = R"doc()doc";

static const char *mkd_doc_morphio_mut_GlialCell_GlialCell_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection = R"doc(Mutable(editable) morphio::MitoSection)doc";

static const char *mkd_doc_morphio_mut_MitoSection_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_MitoSection = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_MitoSection_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_MitoSection_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_appendSection = R"doc(Append a MitoSection)doc";

static const char *mkd_doc_morphio_mut_MitoSection_appendSection_2 =
R"doc(Append a MitoSection

If recursive == true, all descendent mito sections will be appended as
well)doc";

static const char *mkd_doc_morphio_mut_MitoSection_appendSection_3 =
R"doc(Append a MitoSection

If recursive == true, all descendent mito sections will be appended as
well)doc";

static const char *mkd_doc_morphio_mut_MitoSection_children = R"doc(Get the Section children)doc";

static const char *mkd_doc_morphio_mut_MitoSection_diameters = R"doc(@{ Return the diameters of all points of this section)doc";

static const char *mkd_doc_morphio_mut_MitoSection_diameters_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_hasSameShape =
R"doc(Return true if the both sections have the same neuriteSectionIds,
diameters and pathLengths)doc";

static const char *mkd_doc_morphio_mut_MitoSection_id = R"doc(Return the section id)doc";

static const char *mkd_doc_morphio_mut_MitoSection_id_2 = R"doc(@})doc";

static const char *mkd_doc_morphio_mut_MitoSection_isRoot = R"doc(Return true if section is a root section)doc";

static const char *mkd_doc_morphio_mut_MitoSection_mitoPoints = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_mitochondria = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_neuriteSectionIds = R"doc(@{ Return the neurite section Ids of all points of this section)doc";

static const char *mkd_doc_morphio_mut_MitoSection_neuriteSectionIds_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_MitoSection_parent = R"doc(Get the Section parent)doc";

static const char *mkd_doc_morphio_mut_MitoSection_pathLengths =
R"doc(@{ Return the relative distance (between 0 and 1) between the start of
the neuronal section and each point of this mitochondrial section)doc";

static const char *mkd_doc_morphio_mut_MitoSection_pathLengths_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria = R"doc(Mutable(editable) morphio::Mitochondria)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_Mitochondria = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_appendRootSection = R"doc(Append a new root MitoSection)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_appendRootSection_2 =
R"doc(Append a root MitoSection

If recursive == true, all descendent mito sections will be appended as
well)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_appendRootSection_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_breadth_begin =
R"doc(Breadth first iterator

If id == -1, the iteration will be successively performed starting at
each root section)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_breadth_begin_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_breadth_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_buildMitochondria = R"doc(Fill the 'properties' variable with the mitochondria data)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_children = R"doc(Get the Section children)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_children_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_counter = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_depth_begin =
R"doc(Depth first iterator starting at a given section id

If id == -1, the iteration will start at each root section,
successively)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_depth_begin_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_depth_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_isRoot = R"doc(Return true if section is a root section)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_mitoSection = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_parent = R"doc(Return the parent mithochondrial section ID)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_parent_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_register = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_rootSections =
R"doc(Return the list of IDs of all mitochondrial root sections (sections
whose parent ID are -1))doc";

static const char *mkd_doc_morphio_mut_Mitochondria_root_sections = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_section =
R"doc(Get the shared pointer for the given section

Note: multiple morphologies can share the same Section instances.)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_sections = R"doc(Returns the dictionary id -> Section for this tree)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_sections_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_upstream_begin =
R"doc(Upstream first iterator

If id == -1, the iteration will be successively performed starting at
each root section)doc";

static const char *mkd_doc_morphio_mut_Mitochondria_upstream_begin_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Mitochondria_upstream_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_2 = R"doc(Mutable(editable) morphio::Morphology)doc";

static const char *mkd_doc_morphio_mut_Morphology_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_Morphology = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_Morphology_2 =
R"doc(Build a mutable Morphology from an on-disk morphology

options is the modifier flags to be applied. All flags are defined in
their enum: morphio::enum::Option and can be composed.

Example: Morphology("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);)doc";

static const char *mkd_doc_morphio_mut_Morphology_Morphology_3 = R"doc(Build a mutable Morphology from an HighFive::Group)doc";

static const char *mkd_doc_morphio_mut_Morphology_Morphology_4 = R"doc(Build a mutable Morphology from a mutable morphology)doc";

static const char *mkd_doc_morphio_mut_Morphology_Morphology_5 = R"doc(Build a mutable Morphology from a read-only morphology)doc";

static const char *mkd_doc_morphio_mut_Morphology_addAnnotation = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_addMarker = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_annotations = R"doc(Return the annotation objects)doc";

static const char *mkd_doc_morphio_mut_Morphology_appendRootSection =
R"doc(Append the existing morphio::Section as a root section

If recursive == true, all descendent will be appended as well)doc";

static const char *mkd_doc_morphio_mut_Morphology_appendRootSection_2 =
R"doc(Append an existing Section as a root section

If recursive == true, all descendent will be appended as well)doc";

static const char *mkd_doc_morphio_mut_Morphology_appendRootSection_3 = R"doc(Append a root Section)doc";

static const char *mkd_doc_morphio_mut_Morphology_applyModifiers = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_breadth_begin =
R"doc(Breadth first iterator

If id == -1, the iteration will be successively performed starting at
each root section)doc";

static const char *mkd_doc_morphio_mut_Morphology_breadth_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_buildReadOnly = R"doc(Return the data structure used to create read-only morphologies)doc";

static const char *mkd_doc_morphio_mut_Morphology_cellFamily = R"doc(Return the cell family (neuron or glia))doc";

static const char *mkd_doc_morphio_mut_Morphology_cellProperties = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_children = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_connectivity =
R"doc(Return the graph connectivity of the morphology where each section is
seen as a node Note: -1 is the soma node)doc";

static const char *mkd_doc_morphio_mut_Morphology_counter = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_deleteSection =
R"doc(Delete the given section

Will silently fail if the section is not part of the tree

If recursive == true, all descendent sections will be deleted as well
Else, children will be re-attached to their grand-parent)doc";

static const char *mkd_doc_morphio_mut_Morphology_dendriticSpineLevel = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_depth_begin =
R"doc(Depth first iterator starting at a given section id

If id == -1, the iteration will start at each root section,
successively)doc";

static const char *mkd_doc_morphio_mut_Morphology_depth_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_endoplasmicReticulum = R"doc(Return the endoplasmic reticulum container class)doc";

static const char *mkd_doc_morphio_mut_Morphology_endoplasmicReticulum_2 = R"doc(Return the endoplasmic reticulum container class)doc";

static const char *mkd_doc_morphio_mut_Morphology_endoplasmicReticulum_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_eraseByValue = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_err = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_markers = R"doc(Return the markers from the ASC file)doc";

static const char *mkd_doc_morphio_mut_Morphology_mitochondria = R"doc(Return the mitochondria container class)doc";

static const char *mkd_doc_morphio_mut_Morphology_mitochondria_2 = R"doc(Return the mitochondria container class)doc";

static const char *mkd_doc_morphio_mut_Morphology_mitochondria_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_parent = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_register = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_removeUnifurcations =
R"doc(Fixes the morphology single child sections and issues warnings if the
section starts and ends are inconsistent)doc";

static const char *mkd_doc_morphio_mut_Morphology_removeUnifurcations_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_rootSections = R"doc(Returns all section ids at the tree root)doc";

static const char *mkd_doc_morphio_mut_Morphology_rootSections_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_section =
R"doc(Get the shared pointer for the given section

Note: multiple morphologies can share the same Section instances.)doc";

static const char *mkd_doc_morphio_mut_Morphology_sections = R"doc(Returns the dictionary id -> Section for this tree)doc";

static const char *mkd_doc_morphio_mut_Morphology_sections_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_soma =
R"doc(Returns a shared pointer on the Soma

Note: multiple morphologies can share the same Soma instance)doc";

static const char *mkd_doc_morphio_mut_Morphology_soma_2 =
R"doc(Returns a shared pointer on the Soma

Note: multiple morphologies can share the same Soma instance)doc";

static const char *mkd_doc_morphio_mut_Morphology_soma_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Morphology_somaType = R"doc(Return the soma type)doc";

static const char *mkd_doc_morphio_mut_Morphology_version = R"doc(Return the version)doc";

static const char *mkd_doc_morphio_mut_Morphology_write = R"doc(Write file to H5, SWC, ASC format depending on filename extension)doc";

static const char *mkd_doc_morphio_mut_Section = R"doc(Mutable(editable) morphio::Section)doc";

static const char *mkd_doc_morphio_mut_Section_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_Section = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_Section_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_Section_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_appendSection = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_appendSection_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_appendSection_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_breadth_begin = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_breadth_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_children = R"doc(Return a vector of children IDs)doc";

static const char *mkd_doc_morphio_mut_Section_depth_begin = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_depth_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_diameters = R"doc(@{ Return the diameters of all points of this section)doc";

static const char *mkd_doc_morphio_mut_Section_diameters_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_getOwningMorphologyOrThrow =
R"doc(Getter for morphology_; checks the pointer is non-null, throws
otherwise)doc";

static const char *mkd_doc_morphio_mut_Section_hasSameShape =
R"doc(Return true if the both sections have the same points, diameters and
perimeters)doc";

static const char *mkd_doc_morphio_mut_Section_id = R"doc(Return the section ID)doc";

static const char *mkd_doc_morphio_mut_Section_id_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_isHeterogeneous =
R"doc(Return true if the sections of the tree downstream (downstream = true)
or upstream (donwstream = false) have the same section type as the
current section.)doc";

static const char *mkd_doc_morphio_mut_Section_isRoot = R"doc(Return true if section is a root section)doc";

static const char *mkd_doc_morphio_mut_Section_morphology = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_parent =
R"doc(Get the parent ID

Note: Root sections return -1)doc";

static const char *mkd_doc_morphio_mut_Section_perimeters = R"doc(@{ Return the perimeters of all points of this section)doc";

static const char *mkd_doc_morphio_mut_Section_perimeters_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_point_properties = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_points = R"doc(@{ Return the coordinates (x,y,z) of all points of this section)doc";

static const char *mkd_doc_morphio_mut_Section_points_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_properties = R"doc(@{ Return the PointLevel instance that contains this section's data)doc";

static const char *mkd_doc_morphio_mut_Section_properties_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_section_type = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_throwIfNoOwningMorphology =
R"doc(If section is an orphan, in other words it does not belong to an a
morphology)doc";

static const char *mkd_doc_morphio_mut_Section_type = R"doc(@{ Return the morphological type of this section (dendrite, axon, ...))doc";

static const char *mkd_doc_morphio_mut_Section_type_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_upstream_begin = R"doc()doc";

static const char *mkd_doc_morphio_mut_Section_upstream_end = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma = R"doc(Mutable(editable) morphio::Soma)doc";

static const char *mkd_doc_morphio_mut_Soma_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_Soma = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_Soma_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_Soma_3 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_Soma_4 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_center = R"doc(Return the center of gravity of the soma points)doc";

static const char *mkd_doc_morphio_mut_Soma_diameters = R"doc(Return the diameters of all soma points)doc";

static const char *mkd_doc_morphio_mut_Soma_diameters_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_maxDistance =
R"doc(Return the maximum distance between the center of gravity and any of
the soma points)doc";

static const char *mkd_doc_morphio_mut_Soma_point_properties = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_points = R"doc(Return the coordinates (x,y,z) of all soma points)doc";

static const char *mkd_doc_morphio_mut_Soma_points_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_properties = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_properties_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_soma_type = R"doc()doc";

static const char *mkd_doc_morphio_mut_Soma_surface =
R"doc(Return the soma surface Note: the soma surface computation depends on
the soma type)doc";

static const char *mkd_doc_morphio_mut_Soma_type = R"doc(Return the soma type)doc";

static const char *mkd_doc_morphio_mut_Soma_type_2 = R"doc(Return the soma type)doc";

static const char *mkd_doc_morphio_mut_checkDuplicatePoint = R"doc()doc";

static const char *mkd_doc_morphio_mut_diameters = R"doc(@{ Return the diameters of all points of this section)doc";

static const char *mkd_doc_morphio_mut_diameters_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_diameters_3 = R"doc(@{ Return the diameters of all points of this section)doc";

static const char *mkd_doc_morphio_mut_diameters_4 = R"doc()doc";

static const char *mkd_doc_morphio_mut_diff = R"doc(Perform a diff on 2 morphologies, returns True if items differ)doc";

static const char *mkd_doc_morphio_mut_diff_2 = R"doc(Perform a diff on 2 sections, returns True if items differ)doc";

static const char *mkd_doc_morphio_mut_friendDtorForSharedPtrMito = R"doc()doc";

static const char *mkd_doc_morphio_mut_id = R"doc(Return the section id)doc";

static const char *mkd_doc_morphio_mut_id_2 = R"doc(Return the section ID)doc";

static const char *mkd_doc_morphio_mut_modifiers_no_duplicate_point = R"doc(Remove duplicated points)doc";

static const char *mkd_doc_morphio_mut_modifiers_nrn_order = R"doc(Reorders neurites of morphology according to NEURON simulator)doc";

static const char *mkd_doc_morphio_mut_modifiers_soma_sphere =
R"doc(Reduce the soma to a sphere placed at the center of gravity of soma
points and whose radius is the averaged distance between the soma
points and the center of gravity)doc";

static const char *mkd_doc_morphio_mut_modifiers_two_points_sections = R"doc(Only the first and last points of each sections are kept)doc";

static const char *mkd_doc_morphio_mut_neuriteSectionIds = R"doc(@{ Return the neurite section Ids of all points of this section)doc";

static const char *mkd_doc_morphio_mut_neuriteSectionIds_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_operator_lshift = R"doc()doc";

static const char *mkd_doc_morphio_mut_pathLengths =
R"doc(@{ Return the relative distance (between 0 and 1) between the start of
the neuronal section and each point of this mitochondrial section)doc";

static const char *mkd_doc_morphio_mut_pathLengths_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_perimeters = R"doc(@{ Return the perimeters of all points of this section)doc";

static const char *mkd_doc_morphio_mut_perimeters_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_points = R"doc(@{ Return the coordinates (x,y,z) of all points of this section)doc";

static const char *mkd_doc_morphio_mut_points_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_properties = R"doc(@{ Return the PointLevel instance that contains this section's data)doc";

static const char *mkd_doc_morphio_mut_properties_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_rootSections =
R"doc(Return the list of IDs of all mitochondrial root sections (sections
whose parent ID are -1))doc";

static const char *mkd_doc_morphio_mut_sections = R"doc(Returns the dictionary id -> Section for this tree)doc";

static const char *mkd_doc_morphio_mut_type = R"doc(@{ Return the morphological type of this section (dendrite, axon, ...))doc";

static const char *mkd_doc_morphio_mut_type_2 = R"doc()doc";

static const char *mkd_doc_morphio_mut_writer_asc = R"doc(Save morphology in ASC format)doc";

static const char *mkd_doc_morphio_mut_writer_h5 = R"doc(Save morphology in H5 format)doc";

static const char *mkd_doc_morphio_mut_writer_swc = R"doc(Save morphology in SWC format)doc";

static const char *mkd_doc_morphio_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_parent =
R"doc(Return the parent section of this section

Throws:
    MissingParentError is the section doesn't have a parent.)doc";

static const char *mkd_doc_morphio_printError =
R"doc(Print a warning. Raises an error if `set_raise_warnings` was set to
`true`.)doc";

static const char *mkd_doc_morphio_readers_DebugInfo = R"doc(Debug info for error messages)doc";

static const char *mkd_doc_morphio_readers_DebugInfo_2 = R"doc()doc";

static const char *mkd_doc_morphio_readers_DebugInfo_DebugInfo =
R"doc(Constructor

Parameter ``filename``:
    morphology filename.)doc";

static const char *mkd_doc_morphio_readers_DebugInfo_filename = R"doc(Morphology filename)doc";

static const char *mkd_doc_morphio_readers_DebugInfo_getLineNumber = R"doc(Get section's line number within morphology file)doc";

static const char *mkd_doc_morphio_readers_DebugInfo_lineNumbers = R"doc()doc";

static const char *mkd_doc_morphio_readers_DebugInfo_setLineNumber = R"doc(Stores section's line number within morphology file)doc";

static const char *mkd_doc_morphio_readers_ErrorLevel = R"doc(Level of error reporting)doc";

static const char *mkd_doc_morphio_readers_ErrorLevel_ERROR = R"doc(Error)doc";

static const char *mkd_doc_morphio_readers_ErrorLevel_INFO = R"doc(Info)doc";

static const char *mkd_doc_morphio_readers_ErrorLevel_WARNING = R"doc(Warning)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages =
R"doc(Class that can generate error messages and holds a collection of
predefined errors messages)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_2 = R"doc()doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_EOF_IN_NEURITE = R"doc(ASC EOF reached in neurite error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_EOF_REACHED = R"doc(ASC EOF reached error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_EOF_UNBALANCED_PARENS = R"doc(ASC unbalanced parents error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_LINE_NON_PARSABLE = R"doc(Non parsable line error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_MISSING_MITO_PARENT = R"doc(Missing mitochondria parent section error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_MISSING_PARENT = R"doc(Missing section parent error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_MULTIPLE_SOMATA = R"doc(Multiple somas error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_NOT_IMPLEMENTED_UNDEFINED_SOMA = R"doc(Undefined soma error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_ONLY_CHILD_SWC_WRITER = R"doc(Single section child SWC error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_PARSING_POINT = R"doc(Parsing ASC points error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_PERIMETER_DATA_NOT_WRITABLE = R"doc(Can't write perimeter data to SWC, ASC error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_REPEATED_ID = R"doc(Repeated section id error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SELF_PARENT = R"doc(Section self parent error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SOMA_ALREADY_DEFINED = R"doc(Already defined soma error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SOMA_BIFURCATION = R"doc(Bifurcating soma error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SOMA_INVALID_CONTOUR = R"doc(Contour soma must have at least 3 points.)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SOMA_INVALID_SINGLE_POINT = R"doc(Single point soma must have one point)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SOMA_INVALID_THREE_POINT_CYLINDER = R"doc(Multiple points for single point soma)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_SOMA_WITH_NEURITE_PARENT = R"doc(Soma with neurite parent error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_UNCOMPATIBLE_FLAGS = R"doc(Incompatible flags error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_UNEXPECTED_TOKEN = R"doc(Unexpected ASC token error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_UNKNOWN_TOKEN = R"doc(Unknown ASC token error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_UNSUPPORTED_SECTION_TYPE = R"doc(Unsupported morphology section type error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_UNSUPPORTED_SECTION_TYPE_2 = R"doc(Unsupported morphology section type error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_UNSUPPORTED_VASCULATURE_SECTION_TYPE = R"doc(Unsupported vasculature section type error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_VECTOR_LENGTH_MISMATCH = R"doc(Vector length mismatch error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ERROR_WRONG_EXTENSION = R"doc(Wrong morphology file extension error message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ErrorMessages = R"doc()doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_ErrorMessages_2 =
R"doc(Constructor.

Parameter ``uri``:
    path to a morphology file.)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_APPENDING_EMPTY_SECTION = R"doc(Writing empty section warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_DISCONNECTED_NEURITE = R"doc(Writing disconnected neurite warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_MITOCHONDRIA_WRITE_NOT_SUPPORTED = R"doc(Writing of mitochondria is not supported warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_NEUROMORPHO_SOMA_NON_CONFORM = R"doc(Soma does not conform NeuroMorpho warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_NO_SOMA_FOUND = R"doc(Soma not found warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_ONLY_CHILD = R"doc(Writing single child section warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_SOMA_NON_CONTOUR = R"doc(Soma must be a contour for ASC and H5)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_SOMA_NON_CYLINDER_OR_POINT = R"doc()doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_UNDEFINED_SOMA = R"doc(Soma is undefined)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_WRITE_EMPTY_MORPHOLOGY = R"doc(Writing empty morphology warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_WRITE_NO_SOMA = R"doc(Writing without soma warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_WRONG_DUPLICATE = R"doc(Writing wrong duplicate warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_WRONG_ROOT_POINT = R"doc(Wrong root point warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_WARNING_ZERO_DIAMETER = R"doc(Writing zero diameter warning message)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_errorLink = R"doc(Returns a link to a line number within the morphology file)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_errorMsg = R"doc(Generate an error message.)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_isIgnored = R"doc(Is the output of the warning ignored)doc";

static const char *mkd_doc_morphio_readers_ErrorMessages_uri = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample =
R"doc(A sample of section for error reporting, includes its position (line)
within the file.)doc";

static const char *mkd_doc_morphio_readers_Sample_Sample = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_UNKNOWN_ID = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_diameter = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_id = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_lineNumber = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_parentId = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_point = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_type = R"doc()doc";

static const char *mkd_doc_morphio_readers_Sample_valid = R"doc()doc";

static const char *mkd_doc_morphio_set_ignored_warning = R"doc(Set a warning to ignore)doc";

static const char *mkd_doc_morphio_set_ignored_warning_2 = R"doc(Set an array of warnings to ignore)doc";

static const char *mkd_doc_morphio_set_maximum_warnings = R"doc(Set the maximum number of warnings to be printed on screen)doc";

static const char *mkd_doc_morphio_set_raise_warnings = R"doc(Set whether to interpet warning as errors)doc";

static const char *mkd_doc_morphio_vasculature_Section = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_2 = R"doc(Vasculature section)doc";

static const char *mkd_doc_morphio_vasculature_Section_Section = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_Section_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_begin = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_diameters =
R"doc(Return a view
(https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-
intro.md#gslspan-what-is-gslspan-and-what-is-it-for) to this section's
point diameters)doc";

static const char *mkd_doc_morphio_vasculature_Section_end = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_get = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_id = R"doc(Return the ID of this section.)doc";

static const char *mkd_doc_morphio_vasculature_Section_id_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_length = R"doc(Euclidian distance between first and last point of the section)doc";

static const char *mkd_doc_morphio_vasculature_Section_neighbors = R"doc(Returns a list of all neighbors of the section)doc";

static const char *mkd_doc_morphio_vasculature_Section_operator_assign = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_operator_lt = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_points =
R"doc(Return a view
(https://github.com/isocpp/CppCoreGuidelines/blob/master/docs/gsl-
intro.md#gslspan-what-is-gslspan-and-what-is-it-for) to this section's
point coordinates)doc";

static const char *mkd_doc_morphio_vasculature_Section_predecessors = R"doc(Returns a list of predecessors or parents of the section)doc";

static const char *mkd_doc_morphio_vasculature_Section_properties = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_range = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Section_successors = R"doc(Returns a list of successors or children of the section)doc";

static const char *mkd_doc_morphio_vasculature_Section_type =
R"doc(Return the morphological type of this section (artery, vein,
capillary, ...))doc";

static const char *mkd_doc_morphio_vasculature_Vasculature = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_2 =
R"doc(The entry-point class to access vasculature(blood) data

By design, it is the equivalent of the Morphology class but at the
vasculature level. As the Morphology class, it implements a section
accessor and a root section accessor returning views on the Properties
object for the queried vasculature section.)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_Vasculature =
R"doc(@name Read API Open the given source to a vasculature file and parse
it.)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_Vasculature_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_begin = R"doc(graph iterator pointing to the begin)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_diameters = R"doc(Return a vector with all diameters from all sections)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_end = R"doc(graph iterator pointing to the end)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_get = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_operator_assign = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_operator_assign_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_points = R"doc(Return a vector with all points from all sections)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_properties = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_section =
R"doc(Return the Section with the given id.

Throws:
    RawDataError if the id is out of range)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_sectionConnectivity = R"doc(Return a vector with all the connections between sections)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_sectionOffsets =
R"doc(Returns a list with offsets to access data of a specific section in
the points and diameters arrays. p Example: accessing diameters of
n'th section will be located in the Vasculature::diameters array from
diameters[sectionOffsets(n)] to diameters[sectionOffsets(n+1)-1]

Note: for convenience, the last point of this array is the points()
array size so that the above example works also for the last section.)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_sectionTypes = R"doc(Return a vector with the section type of every section)doc";

static const char *mkd_doc_morphio_vasculature_Vasculature_sections = R"doc(Return a vector containing all section objects.)doc";

static const char *mkd_doc_morphio_vasculature_diameters = R"doc(Return a vector with all diameters from all sections)doc";

static const char *mkd_doc_morphio_vasculature_get = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_points = R"doc(Return a vector with all points from all sections)doc";

static const char *mkd_doc_morphio_vasculature_property_Connection =
R"doc(Stores the graph connectivity between the sections. If section1 is
connected to section2, then the last point of section1 and the first
point of section2 must be equal.)doc";

static const char *mkd_doc_morphio_vasculature_property_Diameter = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Point = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties = R"doc(Class that holds all other levels(point, edge, etc.) information)doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_connectivity = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_edgeLevel = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_get = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_get_mut = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_pointLevel = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_predecessors = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_sectionLevel = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_Properties_successors = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_SectionType = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascEdgeLevel = R"doc(Stores edge level information)doc";

static const char *mkd_doc_morphio_vasculature_property_VascEdgeLevel_leakiness = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel = R"doc(Stores point level information. Similar to morphio::PointLevel)doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_VascPointLevel = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_VascPointLevel_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_VascPointLevel_3 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_VascPointLevel_4 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_diameters = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_operator_assign = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascPointLevel_points = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSection = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel = R"doc(stores section level information)doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_diff = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_operator_eq = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_operator_ne = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_predecessors = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_sectionTypes = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_sections = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_VascSectionLevel_successors = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_3 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_4 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_5 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_mut = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_mut_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_mut_3 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_mut_4 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_get_mut_5 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_operator_lshift = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_property_operator_lshift_2 = R"doc()doc";

static const char *mkd_doc_morphio_vasculature_sectionTypes = R"doc(Return a vector with the section type of every section)doc";

static const char *mkd_doc_operator_lshift = R"doc()doc";

static const char *mkd_doc_operator_lshift_2 = R"doc()doc";

static const char *mkd_doc_operator_lshift_3 = R"doc()doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

