# MorphIO [![Build Status](https://travis-ci.com/wizmer/morpho-tool.svg?token=KRP9rHiV52PC6mX3ACXp&branch=master)](https://travis-ci.com/wizmer/morpho-tool)

## Installation

### As a python wheel
The python binding can directly be installed using pip:
```shell
pip install morphio
```

### From sources

#### Dependencies
MorphIO requires the following dependencies:
- cmake >= 3.2
- libhdf5-dev

##### Linux
```sudo apt install cmake libhdf5-dev```

#### Mac OS
```brew install hdf5 cmake```

#### Install as a c++ library

```shell
git clone git@github.com:wizmer/morpho-tool.git --recursive
cd morpho-tool
mkdir build && cd build
cmake ..
make
```

#### Install as a Python package

```shell
pip install git+ssh://git@github.com/wizmer/morpho-tool.git
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

### Read-only API
The read-only API aims at providing better performances as its internal data
representation is contiguous in memory. All accessors return immutable objects and
the mechanism for accessing individual sections
is also simpler than in the read/write API.

In this API, the morphology object is in fact where all data are stored. The
Soma and Section classes are lightweight classes that provide views to the Morphology data.

#### C++
In C++ the API is available right under the `morphio` namespace:
```C++
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>
```

#### Python
In Python the API is available right under the `morphio` module:

```Python
from morphio import Morphology, Section, Soma
```


### Mutable (read/write) API
The read/write API provide a way to read morphologies, edit them and write them to disk.
It offers a tree-centric aproach of the morphology: each section object store its own data and can be seen as a node.
The morphology object simply store the relationship (parent/children) between the different nodes.

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


## Usage
When possible both APIs will try to use the class and function names.

| Immutable C++                      | Immutable Python               | Mutable C++                         | Mutable Python                     |
|------------------------------------|--------------------------------|-------------------------------------|------------------------------------|
| #include <morphio/morphology.h>    | from morphio import Morphology | #include <morphio/mut/morphology.h> | from morphio.mut import Morphology |
|                                    |                                |                                     |                                    |
| using morphio::Morphology;         |                                | using morphio::mut::Morphology      |                                    |
| using morphio::Section;            |                                | using morphio::mut::Section;        |                                    |
| auto m = Morphology("sample.asc"); |                                | auto m = Morphology("sample.asc")   |                                    |
|                                    |                                |                                     |                                    |
| auto roots = m.rootSections()      | roots = m.rootSections         | auto roots = m.rootSections()       | roots = m.rootSections             |
|                                    |                                |                                     |                                    |
| first\_root = roots[0]             | first\_root = roots[0]         | auto first\_root = roots[0]         | first\_root = roots[0]             |
|                                    |                                |                                     |                                    |
