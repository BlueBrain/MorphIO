import numpy as np
import h5py

#from https://bbpteam.epfl.ch/documentation/projects/Morphology%20Documentation/latest/h5v1.html#dendritic-spine-example

with h5py.File('simple-dendtric-spine.h5', 'w') as h5:
    h5.create_dataset('/points',
                      data=np.array([[ 0., 5., 0., 0.1],   # 0 \
                                     [2.4, 9.1, 0., 0.2],  # 1 | s0
                                     [0., 13.2, 0., 0.15], # 2 /
                                     [0., 13.2, 0., 0.2],  # 3 \ s1
                                     [0., 15.9, 0., 2.4],  # 4 /
                                     [0., 13.2, 0., 2.3],  # 5 \
                                     [2.4, 13.2, 0., 2.8], # 6 | s3
                                     [4.03, 13.2, 0., 2.4] # 7 /
                                     ], dtype=np.float32))
    h5.create_dataset('/structure',
                      data=np.array([[0, 2, -1],
                                     [3, 3, 0],
                                     [5, 2, 0]], np.int32))

    h5.create_dataset('/organelles/postsynaptic_density/section_id',
                      data=np.array([1, 2], dtype=np.uint32))

    h5.create_dataset('/organelles/postsynaptic_density/segment_id',
                      data=np.array([0, 1], dtype=np.uint32))

    h5.create_dataset('/organelles/postsynaptic_density/offset',
                      data=np.array([0.8525, 0.9], dtype=np.float32))

    h5.create_group('metadata')

    h5['metadata'].attrs['version'] = np.array([1, 3], dtype=np.uint32)
    h5['metadata'].attrs['cell_family'] = np.array([2], dtype=np.uint32)

