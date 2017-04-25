try:
    import morphotool
except Exception as e:
    print "\033[93m[ERROR] Please run test program from morpho-tool root dir with iPython or set PYTHONPATH to ROOTDIR\033[0m"
    raise e
    
mreader=morphotool.MorphoReader("pybindings/C010306C.h5")

tree=mreader.create_morpho_tree()
print "Tree size:", tree.tree_size

b=tree.get_node(1)
print "Branch length:", b.number_points

#children = b.get_childrens()
#print "Branch childrens in 1", children.count(1)

pts = b.points
print "Branch points:", pts, "length:", len(pts)

bbox = b.bounding_box
print "Bounding Box:", bbox

ls = b.linestring
print "Linestring:", ls, "length:", len(ls)

cp = b.circle_pipe
print "circle pipe:", cp, "length:", len(cp)

