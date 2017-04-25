# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_morpho_operation(_py__base):
    """Python wrapper for class morpho_operation"""
# ----------------------------------------------------------------------------------------------------------------------
    cdef std.shared_ptr[morpho.morpho_operation] _sharedPtr
    cdef morpho.morpho_operation *ptr0(self):
        return <morpho.morpho_operation*> self._ptr

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
        cdef _py_delete_duplicate_point_operation obj = _py_delete_duplicate_point_operation.__new__(_py_delete_duplicate_point_operation)
        obj._ptr = ptr
        if owner: obj._sharedPtr.reset(obj.ptr())
        return obj

    @staticmethod
    cdef _py_delete_duplicate_point_operation from_ref(const morpho.delete_duplicate_point_operation &ref):
        return _py_delete_duplicate_point_operation.from_ptr(<morpho.delete_duplicate_point_operation*> &ref)

    @staticmethod
    cdef _py_delete_duplicate_point_operation from_value(const morpho.delete_duplicate_point_operation &ref):
        cdef morpho.delete_duplicate_point_operation *ptr = new morpho.delete_duplicate_point_operation(ref)
        return _py_delete_duplicate_point_operation.from_ptr(ptr, True)

    @staticmethod
    cdef list vectorPtr2list(std.vector[morpho.delete_duplicate_point_operation*] vec):
        return [_py_delete_duplicate_point_operation.from_ptr(elem) for elem in vec]

    @staticmethod
    cdef list vector2list(std.vector[morpho.delete_duplicate_point_operation] vec):
        return [_py_delete_duplicate_point_operation.from_value(elem) for elem in vec]


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
        cdef _py_duplicate_first_point_operation obj = _py_duplicate_first_point_operation.__new__(_py_duplicate_first_point_operation)
        obj._ptr = ptr
        if owner: obj._sharedPtr.reset(obj.ptr())
        return obj

    @staticmethod
    cdef _py_duplicate_first_point_operation from_ref(const morpho.duplicate_first_point_operation &ref):
        return _py_duplicate_first_point_operation.from_ptr(<morpho.duplicate_first_point_operation*>&ref)

    @staticmethod
    cdef _py_duplicate_first_point_operation from_value(const morpho.duplicate_first_point_operation &ref):
        cdef morpho.duplicate_first_point_operation *ptr = new morpho.duplicate_first_point_operation(ref)
        return _py_duplicate_first_point_operation.from_ptr(ptr, True)

    @staticmethod
    cdef list vectorPtr2list(std.vector[morpho.duplicate_first_point_operation*] vec):
        return [_py_duplicate_first_point_operation.from_ptr(elem) for elem in vec]

    @staticmethod
    cdef list vector2list(std.vector[morpho.duplicate_first_point_operation] vec):
        return [_py_duplicate_first_point_operation.from_value(elem) for elem in vec]



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

