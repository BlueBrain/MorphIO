choco install --yes miniconda3
source C:/Tools/miniconda3/Scripts/activate ;
conda config --set always_yes yes --set changeps1 no
conda update -q conda
conda install -c conda-forge xtl xsimd xtensor
conda install -c conda-forge boost-cpp hdf5 eigen;
