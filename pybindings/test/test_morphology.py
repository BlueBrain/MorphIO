from morphotool import MorphologyDB
morphoDB = MorphologyDB(".")

mor = morphoDB["C010306C"]
s=mor.get_section(1)

