# MorphIO [![Build Status](https://travis-ci.com/wizmer/morpho-tool.svg?token=KRP9rHiV52PC6mX3ACXp&branch=master)](https://travis-ci.com/wizmer/morpho-tool)

## Table of content

* [Installation](#installation)
   * [As a python wheel](#as-a-python-wheel)
   * [From sources](#from-sources)
      * [Dependencies](#dependencies)
         * [Linux](#linux)
         * [Mac OS](#mac-os)
      * [Installation instructions](#installation-instructions)
         * [Install as a c++ library](#install-as-a-c-library)
         * [Install as a Python package](#install-as-a-python-package)
* [Introduction](#introduction)
   * [Quick summary](#quick-summary)
   * [Read-only API](#read-only-api)
      * [C++](#c)
      * [Python](#python)
   * [Mutable (read/write) API](#mutable-readwrite-api)
      * [C++](#c-1)
      * [Python](#python-1)
* [Converter](#converter)
* [Usage](#usage)
   * [Immutable C++](#immutable-c)
   * [Immutable Python](#immutable-python)
   * [Mutable C++](#mutable-c)
      * [Reading morphologies](#reading-morphologies)
      * [Creating morphologies](#creating-morphologies)
   * [Mutable Python](#mutable-python)
      * [Reading morphologies](#reading-morphologies-1)
      * [Creating morphologies](#creating-morphologies-1)
   * [Mitochondria](#mitochondria)



## Installation

### As a python wheel
The python binding can directly be installed using pip:
```shell

pip install --index-url  https://bbpteam.epfl.ch/repository/devpi/bbprelman/dev/+simple/ morphio
```

### From sources

#### Dependencies
MorphIO requires the following dependencies:
- cmake >= 3.2
- libhdf5-dev

##### Linux
```shell
sudo apt install cmake libhdf5-dev
```

##### Mac OS

```shell
brew install hdf5 cmake
```

#### Installation instructions

##### Install as a c++ library

```shell
git clone git@github.com:bluebrain/morphio.git --recursive
cd morphio
mkdir build && cd build
cmake ..
make
```

##### Install as a Python package

```shell
pip install git+ssh://git@github.com/bluebrain/morphio.git
```

## Introduction

MorphIO is a library for reading and writing neuron morphology files.
It supports the following formats:
- SWC
- ASC (aka. neurolucida)
- H5 v1
- H5 v2

It provides 3 classes that are the starting point of every morphology analysis:
- Soma: contains the information related to the soma

- Section: a section is the succession of points between two bifurcation. To the bare minimum
the Section object will contain the section type, the position and diameter of each point.

- Morphology: the morphology object will contain general information about the loaded cell
but will also provide accessors to the different section.

One important concept is that MorphIO is splitted into a *read-only* part and a *read/write* one.

### Quick summary
*C++ vs Python*:
- c++ accessors become python properties
- style: c++ functions are camel case while python ones are snake case

*Read-only vs read/write*
- Hierarchical data (such as parent or children) are accessed at the Section level for the *read-only* API and at the Morphology level for the *read/write* one.
- Hierarchical data accessors return pointer to the concerned section for the *read-only* API, and concerned section IDs for the *read/write* one.

### Read-only API
The read-only API aims at providing better performances as its internal data
representation is contiguous in memory. All accessors return immutable objects and
the mechanism for accessing individual sections
is also simpler than in the read/write API.

Internally, in this API the morphology object is in fact where all data are stored. The
Soma and Section classes are lightweight classes that provide views on the Morphology data.

Hierarchical properties (children/parent properties) can be retrieved from the Section object itself.

#### C++
In C++ the API is available right under the `morphio` namespace:
```C++
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
```

#### Python
In Python the API is available right under the `morphio` module:

```python
from morphio import Morphology, Section, Soma
```

For more convenience, all section data are accessed through properties, such as:
```python
points = section.points
diameters = section.diameters
```

### Mutable (read/write) API
The read/write API provide a way to read morphologies, edit them and write them to disk.
It offers a tree-centric aproach of the morphology: each section object store its own data and can be seen as a node.

The morphology object simply store the hierarchical data (parent/children) between the different nodes. This means that contrary to the read-only API, hierarchical properties must be retrieved through accessors of the Morphology class (and not the Section/Soma ones).

Note: Multiple morphologies can share the same node. This way, cloning a morphology simply means cloning the node relationships but not the data stored in every node.


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

## Converter
MorphIO comes with a file format converter that can go back and forth the following 3 formats:
- asc
- swc
- h5

The converter is compiled as part of the c++ library and will be in the `bin/` folder.
It can be used as:
```shell
./convert inputfile outputfile
```

Note for BBP users: for more information about the intricacy of the format conversion, visit:

<https://bbpteam.epfl.ch/project/issues/browse/NSETM-458>

## Usage
When possible both APIs will try to use the same class and function names.
Here are examples of how to use the various APIs.

### Immutable C++

```cpp
#include <morphio/morphology.h>
#include <morphio/section.h>

int main()
{
    auto m = morphio::Morphology("sample.asc");

    auto roots = m.rootSections();

    auto first_root = roots[0];

    // iterate on sections starting at first_root
    for(auto it = first_root.depth_begin(); it != first_root.depth_end(); ++it) {
        const morphio::Section &section = (*it);

        std::cout << "Section type: " << section.type() << std::endl;
        std::cout << "Section id: " << section.id() << std::endl;
        std::cout << "Parent section id: " << section.parent().id() << std::endl;
        std::cout << "Number of child sections: " << section.children().size() << std::endl;
        std::cout << "X - Y - Z - Diameter" << std::endl;
        for(int i = 0; i<section.points().size(); ++i) {
            std::cout <<
                section.points()[i][0] << ' ' <<
                section.points()[i][1] << ' ' <<
                section.points()[i][2] << ' ' <<
                section.diameters()[i] << std::endl;
        }

        std::cout << std::endl;
    }
}
```


### Immutable Python

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



### Mutable C++

#### Reading morphologies
```cpp
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>

using morphio::mut::Morphology;
using morphio::mut::Section;

int main()
{
    auto m = Morphology("sample.asc");
    auto roots = m.rootSections();
    auto first_root = roots[0];

    // iterate on sections starting at first_root
    for(auto id_it = m.depth_begin(first_root); id_it != m.depth_end(); ++id_it) {
      std::shared_ptr<Section> section = *id_it;

        std::cout << "Section type: " << section->type() << std::endl;
        std::cout << "Section id: " << section->id() << std::endl;
        if(!m.isRoot(section))
          std::cout << "Parent section id: " << m.parent(section) << std::endl;
        std::cout << "Number of child sections: " << m.children(section).size() << std::endl;
        std::cout << "X - Y - Z - Diameter" << std::endl;
        for(int i = 0; i<section->points().size(); ++i) {
            std::cout <<
                section->points()[i][0] << ' ' <<
                section->points()[i][1] << ' ' <<
                section->points()[i][2] << ' ' <<
                section->diameters()[i] << std::endl;
        }

        std::cout << std::endl;
    }
}
```

#### Creating morphologies

Here is a simple example to create a morphology from scratch and writing it to disk

```C++
#include <morphio/mut/morphology.h>

int main()
{
    morphio::mut::Morphology morpho;
    morpho.soma()->points() = {{0, 0, 0}, {1, 1, 1}};
    morpho.soma()->diameters() = {1, 1};

    std::shared_ptr<morphio::mut::Section> section = morpho.appendSection(
        nullptr, // Parent section ID (nullptr = soma)
        morphio::Property::PointLevel(
            {{2, 2, 2}, {3, 3, 3}}, // x,y,z coordinates of each point
            {4, 4}, // diameter of each point
            {5, 5}),
        morphio::SectionType::SECTION_AXON); // (optional) perimeter of each point

    std::shared_ptr<morphio::mut::Section> childSection = morpho.appendSection(
        section,
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

section_id = morpho.append_section(
    None,  # Parent section ID(None=soma)
    PointLevel(
        [[2, 2, 2], [3, 3, 3]],  # x, y, z coordinates of each point
        [4, 4],  # diameter of each point
        [5, 5]),
    SectionType.axon)  # (optional) perimeter of each point

child_section_id = morpho.append_section(
    section_id,
    PointLevel(
        [[3, 3, 3], [4, 4, 4]],
        [4, 4],
        [5, 5])) # section type is omitted -> parent section type will be used

morpho.write("outfile.asc")
morpho.write("outfile.swc")
morpho.write("outfile.h5")
```

### Mitochondria

It is also possible to read and write mitochondria from/to the h5 files (*SWC and ASC are not supported*).
As mitochondria can be represented as trees, one can define the concept of *mitochondrial section*
similar to neuronal section and end up with a similar API. The morphology object has a
*mitochondria* handle method that exposes the basic methods:

- root_sections: returns the section ID of the starting mitochondrial section
of each mitochondrion.
- section(id): returns a given mitochondrial section
- append_section: creates a new mitochondrial section
_ depth_begin: a depth first iterator
_ breadth_begin: a breadth first iterator
_ upstream_begin: an upstream iterator

```python
from morphio.mut import Morphology
from morphio import MitochondriaPointLevel, PointLevel, SectionType

morpho = Morphology()

# A neuronal section that will store mitochondria
section_id = morpho.append_section(
    None,
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
