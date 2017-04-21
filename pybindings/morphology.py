import morphotool
from morphotool import NEURON_STRUCT_TYPE, MorphoTree
from os import path
import numpy
import re

# loader: by default morpho_h5_v1
_h5loader = morphotool.MorphoReader


# Cache properties
class cached_property(object):
    def __init__(self, func):
        self.__doc__ = getattr(func, '__doc__')
        self.func = func

    def __get__(self, obj, cls):
        if obj is None:
            return self
        value = obj.__dict__[self.func.__name__] = self.func(obj)
        # Shallow copy if mutable
        if isinstance(value, list):
            return value[:]
        if isinstance(value, dict):
            return value.copy()
        return value


class Morphology(MorphoTree, object):
    """
    A class representing a Morphology, builds on a Morpho_Tree instance.
    Loader object available in .loader property.
    Methods suffixed with _s return Section objects, which provide extended functionality and 
    are guaranteed to cache results and always return the same object. 
    """
    # morpho_tree cache. Default is None (avoid AttributeError)
    _morpho_tree = None

    def __init__(self, morpho_dir, morpho_name, layer=None, mtype=None):
        self.loader = _h5loader(path.join(morpho_dir, morpho_name+".h5"))
        self._name_attrs = MorphNameExtract(morpho_name)
        self.layer = layer
        self.mtype = mtype
        self.swap(self.loader.create_morpho_tree())

    def __repr__(self):
        return "<%s Morphology>" % (self.label,)

    @property
    def name_attrs(self):
        return self._name_attrs

    @property
    def label(self):
        return self._name_attrs.name

    # @property
    # def type(self):
    #     return

    @property
    def neurites(self):
        return self.axon + self.dendrites

    @property
    def axon(self):
        """Axon sections, as node objects"""
        return self.find_nodes(NEURON_STRUCT_TYPE.axon)

    @property
    def axon_s(self):
        return [s for s in self.sections if s.branch_type == NEURON_STRUCT_TYPE.axon]

    @property
    def dendrites(self):
        return self.basal_dendrites + self.apical_dendrites

    @property
    def dendrites_s(self):
        return self.basal_dendrites_s + self.apical_dendrites_s

    @property
    def basal_dendrites(self):
        return self.find_nodes(NEURON_STRUCT_TYPE.dentrite_basal)

    @property
    def basal_dendrites_s(self):
        return [s for s in self.sections if s.branch_type == NEURON_STRUCT_TYPE.dentrite_basal]

    @property
    def apical_dendrites(self):
        return self.find_nodes(NEURON_STRUCT_TYPE.dentrite_apical)

    @property
    def apical_dendrites_s(self):
        return [s for s in self.sections if s.branch_type == NEURON_STRUCT_TYPE.dentrite_apical]

    def get_section(self, section_id):
        return self.sections[section_id]

    @cached_property
    def sections(self):
        """Nodes converted to Section instances for detailed stats (cached)"""
        return [Section(node, self) for i, node in enumerate(self.all_nodes)]

    @property
    def root(self):
        return self.sections[0]

    # ??
    # @property
    # def path_to_soma(self):
    #     return None
    #
    # @property
    # def mesh(self):
    #     return None


class Section(object):
    def __init__(self, branch_object, tree):    # type: (morphotool.NeuronBranch, morphotool.MorphoTree) -> None
        self.branch_obj = branch_object
        self._tree = tree                # type: morphotool.MorphoTree

    # From branch object
    index         = property(lambda self: self.branch_obj.index)
    branch_type   = property(lambda self: self.branch_obj.branch_type)
    number_points = property(lambda self: self.branch_obj.number_points)
    pointsVector  = property(lambda self: self.branch_obj.pointsVector)
    points        = property(lambda self: self.branch_obj.points)
    radius        = property(lambda self: self.branch_obj.radius)
    bounding_box  = property(lambda self: self.branch_obj.bounding_box)
    linestring    = property(lambda self: self.branch_obj.linestring)
    circle_pipe   = property(lambda self: self.branch_obj.circle_pipe)
    get_segment              = lambda self, n: self.branch_obj.get_segment(n)
    get_segment_bounding_box = lambda self, n: self.branch_obj.get_segment_bounding_box(n)
    get_junction             = lambda self, n: self.branch_obj.get_junction(n)
    get_junction_sphere_bounding_box = lambda self, n: self.branch_obj.get_junction_sphere_bounding_box(n)

    @property
    def children(self):
        return [Section(self._tree.get_section(child_id), self._tree)
                    for child_id in self._tree.get_children(self.index)]

    @property
    def compartments(self):
        return None

    @property
    def compartment(self):
        return None

    @property
    def cross_section(self):
        return None

    @property
    def cross_sections(self):
        return None

    @property
    def num_compartments(self):
        return None

    @property
    def graft(self):
        return None

    @property
    def has_parent(self):
        return None

    @property
    def has_compartments(self):
        return None

    @property
    def is_ancestor_of(self):
        return None

    @property
    def is_descendent_of(self):
        return None

    @property
    def length(self):
        return numpy.linalg.norm(self.points[-1] - self.points[1])

    @property
    def morphology(self):
        return None


    @property
    def parent(self):
        parent_id = self.branch_obj.get_parent()
        if parent_id == 0:
            return None
        return self._tree.get_section(parent_id)

    @property
    def path_to_soma(self):
        return None

    def section_distance(self, segment_id, middle_point=True):
        pass

    @property
    def segments(self):
        return None

    # def move_point(self):
    #     pass
    #
    # def split_segment(self):
    #     pass
    #
    # def resize_diameter(self):
    #     pass

    def __str__(self):
        pass


# class Soma(Section,):
#     def position(self):
#         pass
#
#     def mean_radius(self):
#         pass
#
#     def max_radius(self):
#         pass
#
#     def surface_points(self):
#         pass
#
#     def __str__(self):
#         pass
#


class MorphologyDB(object):
    @staticmethod
    def split_line_to_details(line):
        parts = line.split()
        return parts[0], (int(parts[1]), parts[2])

    def __init__(self, db_path, db_file=None):
        self.db_path = db_path
        self._db = {}
        self._morphos_layer_mtype = {}
        if db_file:
            with open(path.join(db_path, db_file)) as f:
                self._morphos_layer_mtype = { self.split_line_to_details(l) for l in f }


    def __getitem__(self, morpho_name):
        item = self._db.get(morpho_name)
        if not item:
            item = self._db[morpho_name] = Morphology(self.db_path, morpho_name, *self._morphos_layer_mtype.get(item,()))
        return item



# -----------------------------------------------------------
# Aux functions
# -----------------------------------------------------------
class MorphNameExtract(object):
    scaled_mixed_morphname_pattern = re.compile(
        r"dend\-(?P<dend>[\w\-\+]+)_axon\-(?P<axon>[\w\-\+]+)_\-"
        r"_Scale_x(?P<scale_x>[\d]+.[\d]+)_y(?P<scale_y>[\d]+.[\d]+)_z(?P<scale_z>[\d]+.[\d]+)"
    )
    scaled_morphname_pattern = re.compile(
        r"(?P<name>[\w\-\+]+)_\-_Scale_x(?P<scale_x>[\d]+.[\d]+)"
        r"_y(?P<scale_y>[\d]+.[\d]+)_z(?P<scale_z>[\d]+.[\d]+)"
    )
    mixed_morphname_pattern = re.compile(
        r"dend\-(?P<dend>[\w\-\+]+)_axon\-(?P<axon>[\w\-\+]+)"
    )

    def __init__(self, name, mixed=None, scaled=None, cloned=None):
        mixed =False
        cloned=False
        scaled=False

        """ Extraction results """
        if mixed is None:
            self.morphname_extract(name)
        else:
            self.name = name
            self.mixed = mixed
            self.scaled = scaled
            self.cloned = cloned

    def morphname_extract(self, name):
        """ Try and extract the morphname """
        self.cloned = "_-_Clone_" in name
        self.scaled = "_-_Scale_" in name
        self.mixed = "dend-" in name

        if self.cloned:
            name = name[0:str.find(name, '_-_Clone_')]  # Remove from name

        m=None
        attributes=None
        if self.mixed and self.scaled:
            m = self.scaled_mixed_morphname_pattern.match(name)
            attributes = ("dend", "axon", "scale_x", "scale_y", "scale_z")
            name = name[0:str.find(name, '_-_Scale_')]
        elif self.scaled and not self.mixed:
            m = self.scaled_morphname_pattern.match(name)
            attributes = ('name', 'scale_x', 'scale_y', 'scale_z')
        elif self.mixed and not self.scaled:
            m = self.mixed_morphname_pattern.match(name)
            attributes = ('dend', 'axon')

        self.name = name

        if not m:
            raise ValueError("misformatted morph name %s" % name)

        for a in attributes:
            setattr(self, a, m.group(a))

