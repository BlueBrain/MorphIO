# ======================================================================================================================
# Python bindings to namespace morpho::h5_v1
# ======================================================================================================================

# ----------------------------------------------------------------------------------------------------------------------
cdef class MorphoReader(_py__base):
    "Python wrapper class for morpho_reader (ns=morpho::h5_v1)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef unique_ptr[morpho_h5_v1.morpho_reader] _autodealoc
    cdef morpho_h5_v1.morpho_reader *ptr(self):
        return <morpho_h5_v1.morpho_reader*> self._ptr

    def __init__(self, str filename):
        cdef std.string fname
        if isinstance(filename, unicode):
            fname = filename.encode('ascii')
        else:
            fname = filename
        self._ptr = new morpho_h5_v1.morpho_reader(fname)
        self._autodealoc.reset(self.ptr())

    @property
    def points_raw(self ):
        return _Mat_Points.from_value(self.ptr().get_points_raw())

    @property
    def soma_points_raw(self):
        return _Mat_Points.from_value(self.ptr().get_soma_points_raw())

    @property
    def struct_raw(self):
        return _Mat_Index.from_value(self.ptr().get_struct_raw())

    def get_section_range_raw(self, int id_):
        return self.ptr().get_section_range_raw(id_)

    @property
    def filename(self):
        return self.ptr().get_filename()

    def create_morpho_tree(self):
        return MorphoTree.from_move(self.ptr().create_morpho_tree())

    @staticmethod
    cdef MorphoReader from_ptr(morpho_h5_v1.morpho_reader *ptr, bool owner=False):
        cdef MorphoReader obj = MorphoReader.__new__(MorphoReader)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(obj.ptr())
        return obj

    @staticmethod
    cdef MorphoReader from_ref(const morpho_h5_v1.morpho_reader &ref):
        return MorphoReader.from_ptr(<morpho_h5_v1.morpho_reader*>&ref)

    @staticmethod
    cdef MorphoReader from_value(const morpho_h5_v1.morpho_reader &ref):
        cdef morpho_h5_v1.morpho_reader *ptr = new morpho_h5_v1.morpho_reader(ref)
        return MorphoReader.from_ptr(ptr, True)

    @staticmethod
    cdef list vectorPtr2list(std.vector[morpho_h5_v1.morpho_reader*] vec):
        return [MorphoReader.from_ptr(elem) for elem in vec]



# ----------------------------------------------------------------------------------------------------------------------
cdef class MorphoWriter(_py__base):
    "Python wrapper class for morpho_writer (ns=morpho::h5_v1)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef unique_ptr[morpho_h5_v1.morpho_writer] _autodealoc
    cdef morpho_h5_v1.morpho_writer *ptr(self):
        return <morpho_h5_v1.morpho_writer*> self._ptr

    def __init__(self, std.string filename):
        self._ptr = new morpho_h5_v1.morpho_writer(filename)
        self._autodealoc.reset(self.ptr())

    def write(self, MorphoTree tree):
        return self.ptr().write(deref(tree.ptr()))

    @staticmethod
    cdef MorphoWriter from_ptr(morpho_h5_v1.morpho_writer *ptr, bool owner=False):
        cdef MorphoWriter obj = MorphoWriter.__new__(MorphoWriter)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(obj.ptr())
        return obj

    @staticmethod
    cdef MorphoWriter from_ref(const morpho_h5_v1.morpho_writer &ref):
        return MorphoWriter.from_ptr(<morpho_h5_v1.morpho_writer*>&ref)

    @staticmethod
    cdef MorphoWriter from_value(const morpho_h5_v1.morpho_writer &ref):
        cdef morpho_h5_v1.morpho_writer *ptr = new morpho_h5_v1.morpho_writer(ref)
        return MorphoWriter.from_ptr(ptr, True)

    @staticmethod
    cdef list vectorPtr2list(std.vector[morpho_h5_v1.morpho_writer*] vec):
        return [MorphoWriter.from_ptr(elem) for elem in vec]
