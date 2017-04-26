from morphotool import MorphologyDB
morphoDB = MorphologyDB("pybindings")

mor = morphoDB["C010306C"]
s=mor.get_section(1)

