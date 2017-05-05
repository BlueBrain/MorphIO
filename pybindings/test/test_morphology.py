from morphotool import MorphologyDB
import os

morphoDB = MorphologyDB(os.path.dirname(__file__))
mor = morphoDB["C010306C"]
s=mor.get_section(1)
