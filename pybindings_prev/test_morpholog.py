from morphology import MorphologyDB
morphoDB = MorphologyDB("/gpfs/bbp.cscs.ch/release/l2/2012.07.23/morphologies/h5")

mor = morphoDB["C010306C"]
s=mor.get_section(1)

