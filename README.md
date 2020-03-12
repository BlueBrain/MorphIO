# MorphIO [![Build Status](https://travis-ci.com/BlueBrain/MorphIO.svg?token=KRP9rHiV52PC6mX3ACXp&branch=master)](https://travis-ci.com/BlueBrain/MorphIO)

## Table of content

* [Installation](#installation)
   * [Dependencies](#dependencies)
   * [Installation instructions](#installation-instructions)
      * [Install as a C++ library](#install-as-a-c-library)
      * [Install as a Python package](#install-as-a-python-package)
* [Introduction](#introduction)
   * [Quick summary](#quick-summary)
   * [Include/Imports](#includeimports)
   * [Read-only API](#read-only-api)
      * [C++](#c)
      * [Python](#python)
   * [Mutable (read/write) API](#mutable-readwrite-api)
      * [C++](#c-1)
      * [Python](#python-1)
      * [Creating morphologies with the mutable API](#creating-morphologies-with-the-mutable-api)
   * [Mutable Python](#mutable-python)
      * [Reading morphologies](#reading-morphologies)
      * [Creating morphologies](#creating-morphologies)
   * [Opening flags](#opening-flags)
   * [Mitochondria](#mitochondria)
   * [Endoplasmic reticulum](#endoplasmic-reticulum)
   * [Tips](#tips)
      * [Maximum number of warnings](#maximum-number-of-warnings)
* [Specification](#specification)



## Installation

### Dependencies
To build MorphIO from sources, the following dependencies are required:

- cmake >= 3.2
- libhdf5-dev
- A C++11 compiler

Debian:
```shell
sudo apt install cmake libhdf5-dev
```
Red Hat:
```shell
sudo yum install cmake3.x86_64 hdf5-devel.x86_64
```
Max OS:
```shell
brew install hdf5 cmake
```

BB5
```shell
source /opt/rh/rh-python36/enable
module load gcc/5.4.0 nix/cmake/3.9.6
```

### Installation instructions

#### Install as a C++ library

For manual installation:
```shell
git clone git@github.com:bluebrain/morphio.git --recursive
cd morphio
mkdir build && cd build
cmake ..
make install
```

To use the installed library:
```CMake
find_package(MorphIO REQUIRED)

target_link_libraries(mylib MorphIO::morphio)
```

#### Install as a Python package

The python binding can directly be installed using pip:
```shell
pip install morphio
```

## Introduction

MorphIO is a library for reading and writing neuron morphology files.
It supports the following formats:
- SWC
- ASC (aka. neurolucida)
- H5 v1
- H5 v2

It provides 3 C++ classes that are the starting point of every morphology analysis:
- `Soma`: contains the information related to the soma.

- `Section`: a section is the succession of points between two bifurcations. To the bare minimum
the `Section` object will contain the section type, the position and diameter of each point.

- `Morphology`: the morphology object contains general information about the loaded cell
but also provides accessors to the different sections.

One important concept is that MorphIO is split into a *read-only* part and a *read/write* one.

### Quick summary
*C++ vs Python*:
- C++ accessors become Python properties.
- style: C++ functions are camel case while Python ones are snake case.

### Include/imports

- C++ mutable
```C++
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
```

- Python mutable
```python
from morphio import Morphology, Section, Soma
```

- C++ immutable
```C++
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
```

- Python immutable
```python
from morphio.mut import Morphology, Section, Soma
```

### Read-only API
The read-only API aims at providing better performances as its internal data
representation is contiguous in memory. All accessors return immutable objects.

Internally, in this API the morphology object is in fact where all data are stored. The
Soma and Section classes are lightweight classes that provide views on the Morphology data.

For more convenience, all section data are accessed through properties, such as:
```python
points = section.points
diameters = section.diameters
```


#### C++
In C++ the API is available under the `morphio/mut` namespace:
```C++
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
```

#### Python
In Python the API is available under the `morphio.mut` module:

```Python
from morphio.mut import Morphology, Section, Soma
```

### Mutable Read/Write API

#### C++

```cpp
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
```

#### Python

```python
from morphio import Morphology

m = Morphology("sample.asc")
roots = m.rootSections
first_root = roots[0]

# iterate on sections starting at first_root
for section in first_root.iter():
    print("Section type: {}".format(section.type))
    print("Section id: {}".format(section.id))
    print("Parent section id: {}".format(section.parent.id))
    print("Number of child sections: {}".format(len(section.children)))
    print("X - Y - Z - Diameter")

    for point, diameter in zip(section.points, section.diameters):
        print('{} - {}'.format(point, diameter))
```

#### Creating morphologies with the mutable API

Here is a simple example to create a morphology from scratch and write it to disk

```C++
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
```

### Mutable Python

#### Reading morphologies

```python
from morphio.mut import Morphology

m = Morphology("sample.asc")
roots = m.root_sections
first_root = roots[0]

# iterate on sections starting at first_root
for section in m.iter(first_root):
    print("Section type: {}".format(section.type))
    print("Section id: {}".format(section.id))
    if not m.is_root(section):
        print("Parent section id: {}".format(m.parent(section)))
    print("Number of child sections: {}".format(len(m.children(section))))
    print("X - Y - Z - Diameter")

    for point, diameter in zip(section.points, section.diameters):
        print('{} - {}'.format(point, diameter))
```

#### Creating morphologies

Here is a simple example to create a morphology from scratch and writing it to disk

```python
from morphio.mut import Morphology
from morphio import SectionType, PointLevel

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
```
### Opening flags
When opening the file, modifier flags can be passed to alter the morphology representation.
The following flags are supported:

- `morphio::NO_MODIFIER`:
This is the default flag, it will do nothing.
- `morphio::TWO_POINTS_SECTIONS`:
Each section gets reduce to a line made of the first and last point.
- `morphio::SOMA_SPHERE`:
The soma is reduced to a sphere which is the center of gravity of the real soma.
- `morphio::NO_DUPLICATES`:
The duplicate point are not present. It means the first point of each section
is no longer the last point of the parent section.
- `morphio::NRN_ORDER`:
Neurite are reordered according to the
[NEURON simulator ordering](https://github.com/neuronsimulator/nrn/blob/2dbf2ebf95f1f8e5a9f0565272c18b1c87b2e54c/share/lib/hoc/import3d/import3d_gui.hoc#L874)

Multiple flags can be passed by using the standard bit flag manipulation (works the same way in C++ and Python):

C++:
```C++
#include <morphio/Morphology.h>
Morphology("myfile.asc", options=morphio::NO_DUPLICATES|morphio::NRN_ORDER)
```

Python:
```python
from morphio import Morphology, Option
Morphology("myfile.asc", options=Option.no_duplicates|Option.nrn_order)
```

### Mitochondria

It is also possible to read and write mitochondria from/to the h5 files (*SWC and ASC are not supported*).
As mitochondria can be represented as trees, one can define the concept of *mitochondrial section*
similar to neuronal section and end up with a similar API. The morphology object has a
*mitochondria* handle method that exposes the basic methods:

- `root_sections`: returns the section ID of the starting mitochondrial section
of each mitochondrion.
- `section(id)`: returns a given mitochondrial section
- `append_section`: creates a new mitochondrial section
_ `depth_begin`: a depth first iterator
_ `breadth_begin`: a breadth first iterator
_ `upstream_begin`: an upstream iterator

```python
from morphio.mut import Morphology
from morphio import MitochondriaPointLevel, PointLevel, SectionType

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
```

Reading mithochondria from H5 files:

```python
from morphio import Morphology

morpho = Morphology("file_with_mithochondria.h5")

for mitochondrial_section in morpho.mitochondria.root_sections:
    print('{neurite_id}, {relative_path_lengths}, {diameters}'.format(
          neurite_id=mitochondrial_section.neurite_section_ids,
          relative_path_lengths=mitochondrial_section.relative_path_lengths,
          diameters=mitochondrial_section.diameters))

    print("Number of children: {}".format(len(mitochondrial_section.children)))
```
### Endoplasmic reticulum

Endoplasmic reticulum can also be stored and written to H5 file.
The specification is part of the [BBP morphology documentation](https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/h5v1.html)
There is one endoplasmic reticulum object per morphology. It contains 4 attributes. Each attribute is an array and each line
refers to the value of the attribute for a specific neuronal section.

- section_index:
Each row of this dataset represents the index of a neuronal section. Each row of the other properties (eg. volume) refer to the part of the reticulum
present in the corresponding section for each row.

- volume:
One column dataset indexed by section_index. Contains volumes of the reticulum per each corresponding section it lies in.

- surface_area:
Similar to the volume dataset, this dataset represents the surface area of the reticulum in each section in the section_index dataset.

- filament_count:
This 1 column dataset is composed of integers that represent the number of filaments in the segment of the reticulum lying in the section referenced by the corresponding row in the section_index dataset.


#### Reading endoplasmic reticula from H5 files

```python
from morphio import Morphology

morpho = Morphology('/my/file')
reticulum = morpho.endoplasmic_reticulum
print('{indices}, {volumes}, {areas}, {counts}'.format(
    indices=reticulum.section_indices,
    volumes=reticulum.volumes,
    areas=reticulum.surface_areas,
    counts=reticulum.filament_counts))
```

#### Writing endoplasmic reticula from H5 files

```python
neuron = Morphology()

reticulum = neuron.endoplasmic_reticulum
reticulum.section_indices = [1, 1]
reticulum.volumes = [2, 2]
reticulum.surface_areas = [3, 3]
reticulum.filament_counts = [4, 4]
neuron.write('/my/out/file.h5')  # Has to be written to h5
```

### Tips
#### Maximum number of warnings
On can control the maximum number of warnings using the command:
```python

# Will stop displaying warnings after 100 warnings
morphio.set_maximum_warnings(100)

# Will never stop displaying warnings
morphio.set_maximum_warnings(-1)

# Warnings won't be displayed
morphio.set_maximum_warnings(0)
```

# Specification
See https://github.com/BlueBrain/MorphIO/blob/master/doc/specification.md

# Contributing
If you want to improve the project or you see any issue, every contribution is welcome.
Please check the [contribution guidelines](CONTRIBUTING.md) for more information.

# License
MorphIO is licensed under the terms of the GNU Lesser General Public License version 3.
Refer to COPYING.LESSER and COPYING for details.
