try:
    import morphotool
except Exception as e:
    print "\033[93m[ERROR] Please run test program from morpho-tool root dir with iPython or set PYTHONPATH to ROOTDIR\033[0m"
    raise e
    
a=morphotool.Morpho_Reader("/gpfs/bbp.cscs.ch/release/l2/2012.07.23/morphologies/h5/C010306C.h5")

tree=a.create_morpho_tree()
print "Tree size:", tree.get_tree_size()

b=tree.get_branch(1)
print "Branch length:", b.get_size()

children = b.get_childrens()
children.count(1)

pts = b.get_points()

