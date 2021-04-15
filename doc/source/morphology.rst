Morphology Usage
================

Morphology usage can be split into two APIs: mutable and immutable. Mutable is for creating or editing morphologies.
Immutable is for read-only operations. Both are represented in C++ and Python.

Immutable API
-------------

This read-only API aims at providing better performances as its internal data representation is contiguous in
memory. All accessors return immutable objects.

Internally, in this API the morphology object is in fact where all is are stored. The Soma and Section classes
are lightweight classes that provide views on the Morphology data.

**To access it in C++:**

.. code-block:: cpp

   #include <morphio/morphology.h>
   #include <morphio/section.h>
   #include <morphio/soma.h>

**To access it in Python:**

.. code-block:: python

   from morphio import Morphology, Section, Soma


Example of morphology analysis with Immutable API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**C++:**

.. code-block:: cpp

    #include <morphio/morphology.h>
    #include <morphio/section.h>

    int main()
    {
        auto m = morphio::Morphology("sample.asc");

        auto roots = m.rootSections();

        auto first_root = roots[0];

        // iterate on sections starting at first_root
        for (auto it = first_root.depth_begin(); it != first_root.depth_end(); ++it) {
            const morphio::Section &section = *it;

            std::cout << "Section type: " << section.type()
                      << "\nSection id: " << section.id()
                      << "\nParent section id: " << section.parent().id()
                      << "\nNumber of child sections: " << section.children().size()
                      << "\nX - Y - Z - Diameter";
            for (auto i = 0u; i < section.points().size(); ++i) {
                const auto& point = section.points()[i];
                std::copy(point.begin(), point.end(), std::ostream_iterator<float>(std::cout, " "));
                std::cout << '\n' << section.diameters()[i] << '\n';
            }
            std::cout << '\n';
        }
    }


**Python:**

.. code-block:: python

   from morphio import Morphology

   m = Morphology("sample.asc")
   roots = m.root_sections
   first_root = roots[0]

   # iterate on sections starting at first_root
   for section in first_root.iter():
       print("Section type: {}".format(section.type))
       print("Section id: {}".format(section.id))
       if not section.is_root:
          print("Parent section id: {}".format(section.parent.id))
       print("Number of child sections: {}".format(len(section.children)))
       print("X - Y - Z - Diameter")

       for point, diameter in zip(section.points, section.diameters):
           print('{} - {}'.format(point, diameter))


Mutable API
-----------

This API can be as well used for reading morphologies but the main usage of it is creating morphologies
or editing them.

Example of creating morphologies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**C++:**

.. code-block:: cpp

   #include <morphio/mut/morphology.h>

   int main()
   {
       morphio::mut::Morphology morpho;
       morpho.soma()->points() = {{0, 0, 0}, {1, 1, 1}};
       morpho.soma()->diameters() = {1, 1};

       auto section = morpho.appendRootSection(
           morphio::Property::PointLevel(
               {{2, 2, 2}, {3, 3, 3}}, // x,y,z coordinates of each point
               {4, 4}, // diameter of each point
               {5, 5}),
           morphio::SectionType::SECTION_AXON); // (optional) perimeter of each point

       auto childSection = section->appendSection(
           morphio::Property::PointLevel(
               {{3, 3, 3}, {4, 4, 4}},
               {4, 4},
               {5, 5}),
           morphio::SectionType::SECTION_AXON);

       // Writing the file in the 3 formats
       morpho.write("outfile.asc");
       morpho.write("outfile.swc");
       morpho.write("outfile.h5");
   }

**Python:**

.. code-block:: python

   from morphio import PointLevel, SectionType
   from morphio.mut import Morphology

   morpho = Morphology()
   morpho.soma.points = [[0, 0, 0], [1, 1, 1]]
   morpho.soma.diameters = [1, 1]

   section = morpho.append_root_section(
       PointLevel(
           [[2, 2, 2], [3, 3, 3]],  # x, y, z coordinates of each point
           [4, 4],  # diameter of each point
           [5, 5]),
       SectionType.axon)  # (optional) perimeter of each point

   child_section = section.append_section(
       PointLevel(
           [[3, 3, 3], [4, 4, 4]],
           [4, 4],
           [5, 5])) # section type is omitted -> parent section type will be used

   morpho.write("outfile.asc")
   morpho.write("outfile.swc")
   morpho.write("outfile.h5")

Opening flags
-------------

In both types of API when opening the file, modifier flags can be passed to alter the morphology representation.
The following flags are supported:

* ``morphio::NO_MODIFIER``\: This is the default flag, it will do nothing.
* ``morphio::TWO_POINTS_SECTIONS``\: Each section gets reduce to a line made of the first and last
    point.
* ``morphio::SOMA_SPHERE``\: The soma is reduced to a sphere which is the center of gravity of the
    real soma.
* ``morphio::NO_DUPLICATES``\: The duplicate point are not present. It means the first point of
    each section is no longer the last point of the parent section.
* ``morphio::NRN_ORDER``\: Neurite are reordered according to the
    `NEURON simulator ordering <https://github.com/neuronsimulator/nrn/blob/2dbf2ebf95f1f8e5a9f0565272c18b1c87b2e54c/share/lib/hoc/import3d/import3d_gui.hoc#L874>`_

Multiple flags can be passed by using the standard bit flag manipulation (works the same way in C++
and Python):

**C++:**

.. code-block:: cpp

   #include <morphio/Morphology.h>
   Morphology("myfile.asc", options=morphio::NO_DUPLICATES|morphio::NRN_ORDER)

**Python:**

.. code-block:: python

   from morphio import Morphology, Option

   Morphology("myfile.asc", options=Option.no_duplicates|Option.nrn_order)
