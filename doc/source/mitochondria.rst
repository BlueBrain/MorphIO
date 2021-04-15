.. _mitochondria-readme:

Mitochondria
============

It is also possible to read and write mitochondria from/to the h5 files (*SWC and ASC are not
supported*). As mitochondria can be represented as trees, one can define the concept of
*mitochondrial section* similar to neuronal section and end up with a similar API. The morphology
object has a *mitochondria* handle method that exposes the basic methods:

* ``root_sections``: returns the section ID of the starting mitochondrial section of each
    mitochondrion.
* ``section(id)``: returns a given mitochondrial section
* ``append_section``: creates a new mitochondrial section
* ``depth_begin``: a depth first iterator
* ``breadth_begin``: a breadth first iterator
* ``upstream_begin``: an upstream iterator

.. code-block:: python

    from morphio import MitochondriaPointLevel, PointLevel, SectionType
    from morphio.mut import Morphology

    morpho = Morphology()

    # A neuronal section that will store mitochondria
    section = morpho.append_root_section(
      PointLevel([[2, 2, 2], [3, 3, 3]], [4, 4], [5, 5]),
      SectionType.axon)

    # Creating a new mitochondrion
    mito_id = morpho.mitochondria.append_section(
      -1,
      MitochondriaPointLevel([section.id, section.id], # section id hosting the mitochondria point
                             [0.5, 0.6], # relative distance between the start of the section and the point
                             [10, 20] # mitochondria diameters
                             ))

    # Appending a new mitochondrial section to the previous one
    morpho.mitochondria.append_section(
      mito_id, MitochondriaPointLevel([0, 0, 0, 0],
                                      [0.6, 0.7, 0.8, 0.9],
                                      [20, 30, 40, 50]))

    # Iteration works the same as iteration on neuronal sections
    first_root = morpho.mitochondria.root_sections[0]
    for section_id in morpho.mitochondria.depth_begin(first_root):
      section = morpho.mitochondria.section(section_id)
      print('relative_path_length - diameter')
      for relative_path_length, diameter in zip(section.diameters,
                                                section.relative_path_lengths):
          print("{} - {}".format(relative_path_length, diameter))

Reading mithochondria from H5 files:

.. code-block:: python

    from morphio import Morphology

    morpho = Morphology("file_with_mithochondria.h5")

    for mitochondrial_section in morpho.mitochondria.root_sections:
      print('{neurite_id}, {relative_path_lengths}, {diameters}'.format(
            neurite_id=mitochondrial_section.neurite_section_ids,
            relative_path_lengths=mitochondrial_section.relative_path_lengths,
            diameters=mitochondrial_section.diameters))

      print("Number of children: {}".format(len(mitochondrial_section.children)))
