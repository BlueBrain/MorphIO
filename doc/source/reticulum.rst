Endoplasmic reticulum
=====================

Endoplasmic reticulum can also be stored and written to H5 file. The specification is part of the
`BBP morphology documentation <https://developer.humanbrainproject.eu/docs/projects/morphology-documentation/latest/h5v1.html>`__
There is one endoplasmic reticulum object per morphology. It contains 4 attributes. Each attribute
is an array and each line refers to the value of the attribute for a specific neuronal section.

* section_index: Each row of this dataset represents the index of a neuronal section. Each row of
    the other properties (eg. volume) refer to the part of the reticulum present in the
    corresponding section for each row.
* volume: One column dataset indexed by section_index. Contains volumes of the reticulum per each
    corresponding section it lies in.
* surface_area: Similar to the volume dataset, this dataset represents the surface area of the
    reticulum in each section in the section_index dataset.
* filament_count: This 1 column dataset is composed of integers that represent the number of
    filaments in the segment of the reticulum lying in the section referenced by the corresponding
    row in the section_index dataset.

Reading endoplasmic reticula from H5 files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

    from morphio import Morphology

    morpho = Morphology('/my/file')
    reticulum = morpho.endoplasmic_reticulum
    print('{indices}, {volumes}, {areas}, {counts}'.format(
      indices=reticulum.section_indices,
      volumes=reticulum.volumes,
      areas=reticulum.surface_areas,
      counts=reticulum.filament_counts))

Writing endoplasmic reticula from H5 files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

    neuron = Morphology()

    reticulum = neuron.endoplasmic_reticulum
    reticulum.section_indices = [1, 1]
    reticulum.volumes = [2, 2]
    reticulum.surface_areas = [3, 3]
    reticulum.filament_counts = [4, 4]
    neuron.write('/my/out/file.h5')  # Has to be written to h5
