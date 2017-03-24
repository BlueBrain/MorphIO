#!/usr/bin/env python
# -*- coding: utf-8 -*-
""" Tools for dealing with morphologies.

@remarks Copyright (c) BBP/EPFL 2010-2012; All rights reserved.
         Do not distribute without further notice.
"""

import os
import re

from lxml import etree
import numpy

import bbp

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


class MorphNameExtractResult(object):
    """ Extraction results """
    def __init__(self, name, mixed, scaled, cloned):
        self.name = name
        self.mixed = mixed
        self.scaled = scaled
        self.cloned = cloned


def morphname_extract(name):
    """ Try and extract the morphname """
    cloned = "_-_Clone_" in name
    scaled = "_-_Scale_" in name
    mixed = "dend-" in name

    if cloned:
        name = name[0:str.find(name, '_-_Clone_')]  # Remove from name

    result = MorphNameExtractResult(name=name, mixed=mixed, scaled=scaled, cloned=cloned)

    if not scaled and not mixed:
        return result
    elif mixed and scaled:
        m = scaled_mixed_morphname_pattern.match(name)
        attributes = ("dend", "axon", "scale_x", "scale_y", "scale_z")
        result.name = name[0:str.find(name, '_-_Scale_')]
    elif scaled and not mixed:
        m = scaled_morphname_pattern.match(name)
        attributes = ('name', 'scale_x', 'scale_y', 'scale_z')
    elif mixed and not scaled:
        m = mixed_morphname_pattern.match(name)
        attributes = ('dend', 'axon')

    if not m:
        raise ValueError("misformatted morph name %s" % name)

    for a in attributes:
        setattr(result, a, m.group(a))

    return result


class PlacementRules(object):
    """ Manage the placement rules """
    def __init__(self, xml_path):
        self.root = etree.parse(xml_path)
        self.xml_path = xml_path

    def get_mtype_rule_set(self, mtype):
        """ Get the mtype rule set """
        rule_set = self.root.xpath("/placement_rules/mtype_rule_set[@mtype='%s']" % mtype)
        # do some sanity checks
        if len(rule_set) == 0:
            raise ValueError("mtype_rule_set for mtype '%s' not found in '%s'"
                             % (mtype, self.xml_path))
        if len(rule_set) > 1:
            raise ValueError("mtype_rule_set for mtype '%s' not unique in '%s'"
                             % (mtype, self.xml_path))
        rule_set = rule_set[0]
        return rule_set

    def get_mtype_rules(self, mtype):
        """ Try and get mtype rules as a list """
        try:
            rule_set = self.get_mtype_rule_set(mtype)
        except ValueError:
            return []
        return [e for e in rule_set]

    def get_global_rules(self):
        """ returns list of xml elements for the rules in /placement_rules/global_rule_set"""
        return [r for r in self.root.xpath("/placement_rules/global_rule_set")[0]]

    def get_rule(self, mtype, idx):
        """
        Get a particular rule
        """
        #TODO: This has never been used, p doesn't exist - mgevaert
        #r = p.root.xpath("/placement_rules/mtype_rule_set[@mtype='%s']/rule[@id='%s']"
        #                    % (mtype, id))
        r = self.root.xpath("/placement_rules/mtype_rule_set[@mtype='%s']/rule[@idx='%s']"
                            % (mtype, idx))
        if len(r) == 0:
            raise ValueError("rule for mtype '%s' with id '%d' not found in '%s'" %
                             (mtype, idx, self.xml_path))
        if len(r) > 1:
            raise ValueError("rule for mtype '%s' with id '%d' not unique in '%s'" %
                             (mtype, idx, self.xml_path))
        r = r[0]
        return r


class Sections(object):
    """ Sections """
    def __init__(self, bbpsdk_sections, parent_morph):
        self.bbpsdk_sections = bbpsdk_sections
        self.morph = parent_morph

    def seg_points(self): # pylint: disable=R0914
        """ Return the segment points
        as a list of tuples, (x,y,z) where x,y,z are numpy arrays of points per section"""

        ret = []

        sections = self.bbpsdk_sections

        for section in sections:
            segments = section.segments()
            ns = section.segments().size() + 1

            px = numpy.zeros((ns,))
            py = numpy.zeros((ns,))
            pz = numpy.zeros((ns,))

            vx = numpy.zeros((ns,))
            vy = numpy.zeros((ns,))
            vz = numpy.zeros((ns,))

            cs = numpy.zeros((ns,))

            seg_num = 0

            for segment in segments:
                b = segment.begin()
                e = segment.end()
                bc = b.center()
                ec = e.center()
                cs[seg_num] = b.radius()
                px[seg_num] = bc.x()
                py[seg_num] = bc.y()
                pz[seg_num] = bc.z()
                vx[seg_num] = ec.x() - bc.x()
                vy[seg_num] = ec.y() - bc.y()
                vz[seg_num] = ec.z() - bc.z()
                seg_num += 1

            # additional final point
            px[seg_num] = ec.x()
            py[seg_num] = ec.y()
            pz[seg_num] = ec.z()
            cs[seg_num] = e.radius()

            # not so good:
            vx[seg_num] = ec.x() - bc.x()
            vy[seg_num] = ec.y() - bc.y()
            vz[seg_num] = ec.z() - bc.z()

            #data.append((px,py,pz))
            #vec.append((vx,vy,vz))
            #cross_section.append(cs)

            ret.append(((px, py, pz), (vx, vy, vz), cs))

        return ret

    def get_bounding_box(self):
        """ compute and return bounding box (x_min,x_max), (y_min, y_max), (z_min, z_max) """
        hstck = numpy.hstack

        pts = self.seg_points()

        x_min = numpy.inf
        x_max = -numpy.inf
        y_min = numpy.inf
        y_max = -numpy.inf
        z_min = numpy.inf
        z_max = -numpy.inf

        for data, _, _ in pts:
            x, y, z = data
            x_min = numpy.min(hstck((x, x_min)))
            x_max = numpy.max(hstck((x, x_max)))
            y_min = numpy.min(hstck((y, y_min)))
            y_max = numpy.max(hstck((y, y_max)))
            z_min = numpy.min(hstck((z, z_min)))
            z_max = numpy.max(hstck((z, z_max)))

        return (x_min, x_max), (y_min, y_max), (z_min, z_max)


class Morph(object):
    """ Morph object """
    def __init__(self, name, parent_morphdb):
        self.morphdb = parent_morphdb
        self.name = name

    @property
    def bbpsdk_morph(self):
        """ Get morph """
        return self.morphdb.get_bbpsdk_morph(self.name)

    @property
    def annotations(self):
        """ return an lxml.ElementTree object of the annotations xml for this morph """
        xml_path = os.path.join(self.morphdb.annotation_path, self.name + ".xml")
        if not os.path.exists(xml_path):
            print "No annotation file %s" % xml_path
            return None
        return etree.parse(xml_path).getroot()

    @property
    def placement_rule_set(self):
        """ return placement rule_set for this MType"""
        return self.morphdb.placement_rules.get_mtype_rule_set(self.mtype)

    @property
    def placement_rules(self):
        """
        return placement rule_set for this MType and global rules in list
        """

        return (self.morphdb.placement_rules.get_mtype_rules(self.mtype) +
                self.morphdb.placement_rules.get_global_rules())

    def placement_rule_select(self, **kwargs):
        """
        return placement rule_set for this MType (including globals),
        kwargs does selection based on attr_name=key,
        attr_value=value (multiple are logical and)
        """

        rules = self.placement_rules
        selected_rules = []
        for r in rules:
            for k, v in kwargs.iteritems():
                if r.get(k) != v:
                    break
            else:
                selected_rules.append(r)
        return selected_rules

    def annotation_placement_rule_pairs(self):
        """
        return annotation for this MType,
        and associated placement rule in tuple (annotation, placement_rule)
        """

        placement_rules = []
        placement_annotations = self.annotation_select("placement")
        for ann in placement_annotations:
            rules = self.placement_rule_select(id=ann.get("rule"))
            if len(rules) != 1:
                raise ValueError(
                        "Annotation '%s' is not associated\
                         to unique (num_rules=%d) placement rule in mtype_rule_set\
                         for morph '%s', mtype '%s'"
                         % (etree.tostring(ann), len(rules), self.name, self.mtype))
            placement_rules.append((ann, rules[0]))
        return placement_rules

    def placement_annotation_select_on_parent_placement_rule(self, **kwargs):
        """
        return annotation for this MType,
        kwargs does selection based on attr_name=key
        attr_value=value (multiple are logical and)
        """

        pairs = self.annotation_placement_rule_pairs()

        selected_anns = []
        for ann, rule in pairs:
            if rule.getparent().tag != "mtype_rule_set":
                continue
            for k, v in kwargs.iteritems():
                if rule.get(k) != v:
                    break
            else:
                selected_anns.append(ann)

        return selected_anns

    def annotation_select(self, tag, **kwargs):
        """
        return annotation for this MType,
        kwargs does selection on associated placement rule based on attr_name=key,
        attr_value=value (multiple are logical and)
        """
        anns = self.annotations
        assert anns is not None, "No annotations!!"
        selected_anns = []
        for a in anns:
            if a.tag != tag:
                continue
            for k, v in kwargs.iteritems():
                if a.get(k) != v:
                    break
            else:
                selected_anns.append(a)

        return selected_anns

    @property
    def mtype(self):
        """ Get mtypes """
        return self.morphdb.mtypes[self.name]

    @property
    def layers(self):
        """ Get layers """
        return self.morphdb.layers[self.name]

    @property
    def axon(self):
        """ Return axon Sections """
        return Sections(self.bbpsdk_morph.axon(), self)

    @property
    def dendrites(self):
        """ Return dentrites Sections """
        return Sections(self.bbpsdk_morph.dendrites(), self)

    def get_bounding_box(self):
        """ Return Sections bounding box """
        s = Sections(self.bbpsdk_morph.neurites(), self)
        return s.get_bounding_box()


class MorphDB(object):
    """ Various MorphDB helper functions """
    def __init__(self, morphs_db, placement_rules=None, annotation_path=None):
        self.placement_rules = placement_rules
        self.root = os.path.split(morphs_db)[0]
        self.morphs_db = morphs_db

        f = file(self.morphs_db)

        # map names to mtype
        self.mtypes = {}
        self.all_mtypes = {}

        self.layers = {}
        self.layer_mtypes = {}
        self.layer_names = {}
        self.placement_scores = {}
        self.neurondb_lines = {}
        for l in f:
            l = l.strip()
            if not l:
                continue
            columns = l.split()
            name, layer, mtype = columns[:3]
            # don't overrite mtype
            self.mtypes.setdefault(name, mtype)
            self.all_mtypes.setdefault(name, []).append(mtype)

            ndb_lines = self.neurondb_lines.setdefault(name, [])
            ndb_lines.append(l)

            layer_names_list = self.layer_names.setdefault(int(layer), set([]))
            layer_names_list.add(name)

            # does the neuronDB have v4 placement scores?
            # len(columns)==7 -> new<date>NeuronDB.dat
            # len(columns)>7 -> v4_<date>NeuronDB.dat
            if len(columns) > 7:
                self.placement_scores[(name, int(layer))] = map(float, columns[5:])

            # add mtype to list of mtypes per layer
            layer_mtypes_list = self.layer_mtypes.setdefault(layer, set([]))
            layer_mtypes_list.add(mtype)

            layers_list = self.layers.setdefault(name, [])
            layers_list.append(layer)

        f.close()

        # this is the bbp_sdk loading cache
        #self.morphies=bbp.Morphologies()
        self.morphies = None
        # this maintain the mapping of name to morph
        self.morph_map = {}

        if annotation_path is None:
            self.annotation_path = os.path.join(self.root, "annotations")
        else:
            self.annotation_path = annotation_path

    def _update_morph_map(self):
        """ Update morph_map from morphies """
        # unfortunately, bbp-sdk does not return morphs in the order specified
        # in the label set
        self.morph_map = {}
        for m in self.morphies:
            self.morph_map[m.label()] = m

    def get_bbpsdk_morph(self, name):
        """ Load a morph """
        # is it already loaded?
        if name in self.morph_map:
            return self.morph_map[name]
        else:
            # load the morph
            self.load([name])
            return self.morph_map[name]

    @property
    def names(self):
        """ returns morph names in the db """
        return self.mtypes.keys()

    def get_one(self, morph_name, load=False):
        """ return morph object for morph_name """
        if morph_name not in self.mtypes:
            raise KeyError("requested morph '%s' not in db" % morph_name)
        if load:
            self.load([morph_name])
        return Morph(morph_name, self)

    def get_all(self):
        """ return list of all Morphs in db """
        return self.get(self.names)

    def get(self, morph_names, load=False):
        """ return list of morph objects for morph_names """
        for morph_name in morph_names:
            if morph_name not in self.mtypes:
                raise KeyError("requested morph '%s' not in db" % morph_name)

        if load:
            self.load(morph_names)
        return [Morph(name, self) for name in morph_names]

    def get_mtype(self, mtype_filter, load=False):
        """ load all morphs of mtype == mtype_filter, return as list """
        morph_names = []
        for name, mtypes in self.all_mtypes.iteritems():
            if mtype_filter in mtypes:
                morph_names.append(name)
        return self.get(morph_names, load)

    def morphs_missing_annotations(self):
        """ get names of all morphs missing annotations """
        missing = []
        for name, mtype in self.mtypes.iteritems():
            if not os.path.exists(
                    os.path.join(self.annotation_path, name + ".xml")):
                missing.append((name, mtype))
        return missing

    # TODO
    #def unload(self, morph_names):

    def _get_morph_labels_old(self, morph_names):
        """get the set of morphology lables using the old BBPSDK 2012 API"""
        # TODO remove (involves loosing support for old BBSDK, delayed until SPIND-49 is resolved)
        labels = bbp.Label_Set()  # # pylint: disable=E1101
        for name in morph_names:
            if name not in self.morph_map:
                labels.insert(name)
        return labels

    def _get_morph_labels(self, morph_names):
        """get the set of morphology lables"""
        labels = set()
        for name in morph_names:
            if name not in self.morph_map:
                labels.add(name)
        return labels

    def load(self, morph_names):
        """ load a list of morph names """

        r = bbp.Morphology_Reader()
        # TODO revert to load from v2 after BBPSDK-295 has been fixed
        r.open(os.path.join(self.root, "v1"))

        try:
            labels = self._get_morph_labels_old(morph_names)
        except AttributeError:
            labels = self._get_morph_labels(morph_names)

        if self.morphies is None:
            self.morphies = bbp.Morphologies()

        r.read(self.morphies, labels)
        # get the first (and only) morphology in the Morphologies object.
        r.close()

        # TODO: optimization, this could be an incremental update
        self._update_morph_map()
