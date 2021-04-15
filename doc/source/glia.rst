Glia
====

MorphIO also support reading and writing glia (such as astrocytes) from/to disk according to the
`H5 specification <https://developer.humanbrainproject.eu/docs/projects/morphology-documentation/latest/h5v1.html>`__

.. code-block:: python

    import morphio

    # Immutable
    immutable_glia = morphio.GlialCell("astrocyte.h5")

    # Mutable
    empty_glia = morphio.mut.GlialCell()
    mutable_glia = morphio.mut.GlialCell("astrocyte.h5")
