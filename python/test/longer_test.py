# Package morphotool
# morphotool.MorphoReader  morphotool.MorphoWriter  morphotool.Morphology
# morphotool.MorphologyDB  morphotool.morphology    morphotool.morphotool

from morphotool import MorphoReader, MorphologyDB
reader = MorphoReader("morpho-tool/pybindings/test/C010306C.h5")
reader.points_raw
# Out[3]:
# <<type 'morphotool.morphotool._Mat_Points'> object
# [[ 1.22599994  5.30685726  0.          0.        ]
#  [ 0.74599999  5.40685717  0.          0.        ]
#  [-0.72399998  5.58685747  0.          0.        ]]...
#  (Full numpy array accessible at .nparray) >

mtree = reader.create_morpho_tree()
# mtree.add_node      mtree.bounding_box  mtree.find_nodes    mtree.get_node      mtree.soma          mtree.transform
# mtree.all_nodes     mtree.copy_node     mtree.get_children  mtree.get_parent    mtree.swap          mtree.tree_size

# Nodes
mtree.all_nodes[:2] + mtree.all_nodes[-2:]
# Out[6]:
# [<MorphoNode::soma nr.0>,
#  <MorphoNode::axon nr.1>,
#  <MorphoNode::dentrite_basal nr.103>,
#  <MorphoNode::dentrite_basal nr.104>]

# BBox props
mtree.bounding_box
#Out: <_Box [(-212.090, -367.765, -312.120) - (586.836, 211.402, 264.537)]>

node = mtree.get_node(5)
# node.bounding_box                      node.get_segment                       node.number_points
# node.section_type                       node.get_segment_bounding_box          node.points
# node.circle_pipe                       node.index                             node.pointsVector
# node.get_junction                      node.is_of_type                        node.radius
# node.get_junction_sphere_bounding_box  node.linestring


from morphotool.morphotool import Transforms
del_op=Transforms.Delete_Duplicate_Point_Operation()

mtree.transform([del_op])
# Out[11]: <morphotool.morphotool.MorphoTree at 0x7f89376989f0>

# --------------
# High level API
# --------------
morphoDB = MorphologyDB("morpho-tool/pybindings/test")
mor = morphoDB["C010306C"]
# mor.RawNodesContext   mor.basal_dendrites   mor.get_children      mor.layer             mor.raw               mor.tree_size
# mor.add_node          mor.bounding_box      mor.get_node          mor.loader            mor.sections
# mor.all_nodes         mor.copy_node         mor.get_parent        mor.mtype             mor.soma
# mor.apical_dendrites  mor.dendrite_nodes    mor.get_section       mor.name_attrs        mor.swap
# mor.axon              mor.find_nodes        mor.label             mor.neurites          mor.transform
