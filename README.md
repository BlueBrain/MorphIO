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
