from __future__ import print_function
import morphotool
import os

_CUR_DIR = os.path.dirname(__file__)
_H5V1_MORPHO_TEST_FILE = os.path.join(_CUR_DIR, "C010306C.h5")
_DEBUG = False

mreader=morphotool.MorphoReader(_H5V1_MORPHO_TEST_FILE)

tree=mreader.create_morpho_tree()
print("Tree size:", tree.tree_size)

b=tree.get_node(1)
if _DEBUG:
    print("Section length:", b.number_points)

#children = b.get_childrens()
#print "Section childrens in 1", children.count(1)

pts = b.points
if _DEBUG:
    print("Section points:", pts, "length:", len(pts))

bbox = b.bounding_box
if _DEBUG:
    print("Bounding Box:", bbox)

ls = b.linestring
if _DEBUG:
    print("Linestring:", ls)

cp = b.circle_pipe
if _DEBUG:
    print("circle pipe:", cp, "length:", len(cp))

