from .statics cimport morpho_morpho_mesher

# ----------------------------------------------------------------------------------------------------------------------
cdef class _py_morpho_mesher(_py__base):
    "Python wrapper class for morpho_mesher (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef unique_ptr[morpho.morpho_mesher] _autodealoc
    cdef morpho.morpho_mesher *ptr(self):
        return <morpho.morpho_mesher*> self._ptr

    def __init__(self, MorphoTree tree, std.string output_mesh_file):
        self._ptr = new morpho.morpho_mesher(tree._sharedPtr, output_mesh_file)
        self._autodealoc.reset(self.ptr())

    def set_mesh_tag(self, int arg0, bool value):
        return self.ptr().set_mesh_tag(<morpho_morpho_mesher.mesh_tag> arg0, value)

    def set_error_bound(self, double inv_error):
        return self.ptr().set_error_bound(inv_error)

    def set_face_size(self, double face_size):
        return self.ptr().set_face_size(face_size)

    def execute(self, ):
        return self.ptr().execute()

    @staticmethod
    cdef _py_morpho_mesher from_ptr(morpho.morpho_mesher *ptr, bool owner=False):
        cdef _py_morpho_mesher obj = _py_morpho_mesher.__new__(_py_morpho_mesher)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(obj.ptr())
        return obj

    @staticmethod
    cdef _py_morpho_mesher from_ref(const morpho.morpho_mesher &ref):
        return _py_morpho_mesher.from_ptr(<morpho.morpho_mesher*>&ref)

    @staticmethod
    cdef _py_morpho_mesher from_value(const morpho.morpho_mesher &ref):
        cdef morpho.morpho_mesher *ptr = new morpho.morpho_mesher(ref)
        return _py_morpho_mesher.from_ptr(ptr, True)

    @staticmethod
    cdef list vectorPtr2list(std.vector[morpho.morpho_mesher*] vec):
        return [_py_morpho_mesher.from_ptr(elem) for elem in vec]
