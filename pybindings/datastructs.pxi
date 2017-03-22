# -------------------------------------------------------------------------------------------------------------
cdef class Box(_py__base):
    cdef std.unique_ptr[morpho.box] _autodealoc
    cdef morpho.box* ptr(self):
        return < morpho.box * > self._ptr

    @property
    def min_corner(self):
        cdef morpho.point p = self.ptr().min_corner()
        cdef const double* pts = p.data()
        return [pts[0], pts[1], pts[2]]

    @property
    def max_corner(self):
        cdef morpho.point p = self.ptr().max_corner()
        cdef const double* pts = p.data()
        return [pts[0], pts[1], pts[2]]

    @staticmethod
    cdef Box from_ptr(morpho.box *ptr, bool owner=False):
        cdef Box obj = Box.__new__(Box)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef Box from_value(const morpho.box &ref):
        cdef morpho.box *ptr = new morpho.box(ref)
        return Box.from_ptr(ptr, True)


# # ----------------------------------------------------------------------------------------------------------------------
# cdef class Vector(_py__base):
#     cdef morpho.vector * ptr(self):
#         return < morpho.vector *> self._ptr
#
#     @staticmethod
#     cdef Vector from_ptr(morpho.vector *ptr):
#         cdef Vector obj = Vector.__new__(Vector)
#         obj._ptr = ptr
#         return obj
#
#     @staticmethod
#     cdef Vector from_ref(const morpho.vector &ref):
#         return Vector.from_ptr(<morpho.vector*>&ref)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Linestring(_py__base):
    cdef std.unique_ptr[morpho.linestring] _autodealoc
    cdef morpho.linestring * ptr(self):
        return < morpho.linestring *> self._ptr

    @staticmethod
    cdef Linestring from_ptr(morpho.linestring *ptr, bool owner=False):
        cdef Linestring obj = Linestring.__new__(Linestring)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef Linestring from_value(const morpho.linestring &ref):
        cdef morpho.linestring* ptr = new morpho.linestring(ref)
        return Linestring.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Cone(_py__base):
    cdef std.unique_ptr[morpho.cone] _autodealoc
    cdef morpho.cone * ptr(self):
        return <morpho.cone *> self._ptr

    @property
    def center(self):
        cdef morpho.point p = self.ptr().get_center()
        cdef const double* pts = p.data()
        return [pts[0], pts[1], pts[2]]

    @property
    def radius(self):
        return self.ptr().get_radius()

    @staticmethod
    cdef Cone from_ptr(morpho.cone *ptr, bool owner=False):
        cdef Cone obj = Cone.__new__(Cone)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef Cone from_value(const morpho.cone &ref):
        cdef morpho.cone* ptr = new morpho.cone(ref)
        return Cone.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Sphere(_py__base):
    cdef std.unique_ptr[morpho.sphere] _autodealoc
    cdef morpho.sphere * ptr(self):
        return < morpho.sphere *> self._ptr

    @property
    def center(self):
        cdef morpho.point p = self.ptr().get_center()
        cdef const double* pts = p.data()
        return [pts[0], pts[1], pts[2]]

    @property
    def radius(self):
        return self.ptr().get_radius()

    @staticmethod
    cdef Sphere from_ptr(morpho.sphere *ptr, bool owner=False):
        cdef Sphere obj = Sphere.__new__(Sphere)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef Sphere from_value(const morpho.sphere &ref):
        cdef morpho.sphere* ptr = new morpho.sphere(ref)
        return Sphere.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Morpho_Node(_py__base):
    "Python wrapper class for morpho_node (ns=morpho)"
# ----------------------------------------------------------------------------------------------------------------------
    cdef std.unique_ptr[morpho.morpho_node] _autodealoc
    cdef morpho.morpho_node *ptr(self):
        return <morpho.morpho_node*> self._ptr


    def __init__(self, int my_node_type):
        self._ptr = new morpho.morpho_node(<morpho.branch_type> my_node_type)
        self._autodealoc.reset(self.ptr())

    def get_type(self, ):
        return self.ptr().get_type()

    @staticmethod
    cdef Morpho_Node from_ptr(morpho.morpho_node *ptr):
        cdef Morpho_Node obj = Morpho_Node.__new__(Morpho_Node)
        obj._ptr = ptr
        return obj

    @staticmethod
    cdef Morpho_Node from_ref(const morpho.morpho_node &ref):
        return Morpho_Node.from_ptr(<morpho.morpho_node*>&ref)

    @staticmethod
    cdef list vector2list( std.vector[morpho.morpho_node*] vec ):
        return [ Morpho_Node.from_ptr(elem) for elem in vec ]



# Data Structures wrappers
cdef class Mat_Points(_py__base):
    cdef object nparray

    cdef morpho.mat_points * ptr(self):
        return <morpho.mat_points *> self._ptr

    def __repr__(self):
        leng = len(self.nparray)
        if leng>3: leng=3
        return """<morphotool.Mat_Points object:
%s...
[Full np.array accesible at object.np_array]>""" % (repr(self.nparray[:leng]),)

    # Pass on the array API
    def __getitem__(self, item):
        return self.nparray.__getitem__(item)

    # Pass on the iterator API
    def __iter__(self):
        return iter(self.nparray)

    @property
    def np_array(self):
        return self.nparray

    @staticmethod
    cdef Mat_Points from_ptr(morpho.mat_points * matpoints):
        cdef Mat_Points obj = Mat_Points()
        obj._ptr = matpoints

        # Create a numpy array (memviews dont expose no nicely to python)
        cdef np.npy_intp[2] dim
        dim[0] = matpoints.size1()
        dim[1] = matpoints.size2()
        cdef np.ndarray[np.double_t, ndim=2] arr = np.PyArray_SimpleNewFromData(2, dim, np.NPY_DOUBLE, matpoints.data().begin())
        obj.nparray = arr

        return obj

    @staticmethod
    cdef Mat_Points from_ref(const morpho.mat_points &ref):
        return Mat_Points.from_ptr(<morpho.mat_points*>&ref)


cdef class Vec_Double(_py__base):
    cdef morpho.vec_double* ptr(self):
        return <morpho.vec_double *> self._ptr

    @staticmethod
    cdef Vec_Double from_ptr(morpho.vec_double *ptr):
        cdef Vec_Double obj = Vec_Double.__new__(Vec_Double)
        obj._ptr = ptr
        return obj

    @staticmethod
    cdef Vec_Double from_ref(const morpho.vec_double &ref):
        return Vec_Double.from_ptr(<morpho.vec_double*>&ref)

