# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_morpho_operation(_py__base):
    """Python wrapper for class morpho_operation"""
# ----------------------------------------------------------------------------------------------------------------------
    cdef std.shared_ptr[morpho.morpho_operation] _sharedPtr
    cdef morpho.morpho_operation *ptr0(self):
        return <morpho.morpho_operation*> self._ptr

    @staticmethod
    cdef _py_morpho_operation from_ptr_base(type subcls, morpho.morpho_operation *ptr, bool owner=False):
        cdef _py_morpho_operation obj = _py__base._create(subcls, ptr)
        if owner: obj._sharedPtr.reset(obj.ptr0())
        return obj

    @property
    def name(self):
        return self.ptr0().name()

    def apply(self, MorphoTree tree):
        MorphoTree.from_value(self.ptr0().apply(deref(tree.ptr())))


# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_delete_duplicate_point_operation(_py_morpho_operation):
    "Python wrapper class for delete_duplicate_point_operation (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef morpho.delete_duplicate_point_operation *ptr(self):
        return <morpho.delete_duplicate_point_operation*> self._ptr

    def __init__(self, ):
        self._ptr = new morpho.delete_duplicate_point_operation()
        self._sharedPtr.reset(self.ptr())

    @staticmethod
    cdef _py_delete_duplicate_point_operation from_ptr(morpho.delete_duplicate_point_operation *ptr, bool owner=False):
        return _py_morpho_operation.from_ptr_base(_py_delete_duplicate_point_operation, ptr, owner)


# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_duplicate_first_point_operation(_py_morpho_operation):
    "Python wrapper class for duplicate_first_point_operation (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef morpho.duplicate_first_point_operation *ptr(self):
        return <morpho.duplicate_first_point_operation*> self._ptr

    def __init__(self, ):
        self._ptr = new morpho.duplicate_first_point_operation()
        self._sharedPtr.reset(self.ptr())

    @staticmethod
    cdef _py_duplicate_first_point_operation from_ptr(morpho.duplicate_first_point_operation *ptr, bool owner=False):
        return _py_morpho_operation.from_ptr_base(_py_duplicate_first_point_operation, ptr, owner)


# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_soma_sphere_operation(_py_morpho_operation):
    "Python wrapper class for duplicate_first_point_operation (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef morpho.soma_sphere_operation *ptr(self):
        return <morpho.soma_sphere_operation*> self._ptr

    def __init__(self, ):
        self._ptr = new morpho.soma_sphere_operation()
        self._sharedPtr.reset(self.ptr())

    @staticmethod
    cdef _py_soma_sphere_operation from_ptr(morpho.soma_sphere_operation *ptr, bool owner=False):
        return _py_morpho_operation.from_ptr_base(_py_soma_sphere_operation, ptr, owner)


# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_simplify_section_extreme_operation(_py_morpho_operation):
    "Python wrapper class for duplicate_first_point_operation (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef morpho.simplify_section_extreme_operation *ptr(self):
        return <morpho.simplify_section_extreme_operation*> self._ptr

    def __init__(self, ):
        self._ptr = new morpho.simplify_section_extreme_operation()
        self._sharedPtr.reset(self.ptr())

    @staticmethod
    cdef _py_simplify_section_extreme_operation from_ptr(morpho.simplify_section_extreme_operation *ptr, bool owner=False):
        return _py_morpho_operation.from_ptr_base(_py_simplify_section_extreme_operation, ptr, owner)


# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_transpose_operation(_py_morpho_operation):
    "Python wrapper class for duplicate_first_point_operation (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef morpho.transpose_operation *ptr(self):
        return <morpho.transpose_operation*> self._ptr

    def __init__(self, vector_transpose, quaternion_transpose):
        cdef morpho.vector3d vec3
        cdef morpho.quaternion3d vec4
        if len(vector_transpose) !=3:
            raise ValueError("Vector transpose must have 3 doubles")
        if len(quaternion_transpose) !=4:
            raise ValueError("Quaternion transpose must have 4 doubles")

        cdef int i = 0
        for i in range(3):
            vec3[i] = vector_transpose[i]
        for i in range(4):
            vec4[i] = quaternion_transpose[i]

        self._ptr = new morpho.transpose_operation(vec3, vec4)
        self._sharedPtr.reset(self.ptr())

    @staticmethod
    cdef _py_transpose_operation from_ptr(morpho.transpose_operation *ptr, bool owner=False):
        return _py_morpho_operation.from_ptr_base(_py_transpose_operation, ptr, owner)


# ----------------------------------------------------------------------------------------------------------------------
cdef class SpatialIndex(_py__base):
    "Python wrapper class for spatial_index (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef unique_ptr[morpho.spatial_index] _autodealoc
    cdef morpho.spatial_index *ptr(self):
        return <morpho.spatial_index*> self._ptr

    def __init__(self, ):
        self._ptr = new morpho.spatial_index()
        self._autodealoc.reset(self.ptr())

    def add_morpho_tree(self, MorphoTree tree):
        return self.ptr().add_morpho_tree(tree._sharedPtr)

    def is_within(self, _Point p):
        return self.ptr().is_within(deref(p.ptr()))

    @staticmethod
    cdef SpatialIndex from_ptr(morpho.spatial_index *ptr, bool owner=False):
        cdef SpatialIndex obj = SpatialIndex.__new__(SpatialIndex)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(obj.ptr())
        return obj

    @staticmethod
    cdef SpatialIndex from_ref(const morpho.spatial_index &ref):
        return SpatialIndex.from_ptr(<morpho.spatial_index*>&ref)

    @staticmethod
    cdef list vectorPtr2list(std.vector[morpho.spatial_index*] vec):
        return [SpatialIndex.from_ptr(elem) for elem in vec]


# ************************************
# Class-Namespace alias
# ************************************
class Transforms:
    Delete_Duplicate_Point = _py_delete_duplicate_point_operation
    Duplicate_First_Point = _py_duplicate_first_point_operation
    Soma_Sphere = _py_soma_sphere_operation
    Simplify_Branch_Extreme = _py_simplify_section_extreme_operation
    Transpose = _py_transpose_operation

