from libc.stdio cimport printf
from libc.stdlib cimport malloc

# cython hack for having integer template
cdef extern from *:
    ctypedef int zero_t "0"

cdef class Point(_py__base):
    cdef std.unique_ptr[morpho.point] _autodealoc
    cdef morpho.point* ptr(self):
        return <morpho.point*>self._ptr

    def __init__(self, std.vector[double] p):
        self._ptr = new morpho.point(p[0], p[1], p[2])
        self._autodealoc.reset(self.ptr())

    def close_to(self, Point other):
        return self.ptr().close_to(deref(other.ptr()))

    def __getattr__(self, int item):
        cdef const double *_data = self.ptr().data()
        return _data[item]

    def __setattr__(self, int item, double value):
        cdef double *_data = <double*>self.ptr().data()
        _data[item] = value

    def __repr__(self):
        return repr(list(self._data))

    @staticmethod
    cdef Point from_ptr(morpho.point* ptr, bool owner=False):
        cdef Point obj = Point.__new__(Point)
        obj._ptr = ptr
        if owner:
            obj._autodealoc.reset(obj.ptr())
        return obj

    @staticmethod
    cdef Point from_value(const morpho.point &ref):
        cdef morpho.point *p = new morpho.point(ref)
        return Point.from_ptr(p, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Box(_py__base):
    cdef std.unique_ptr[morpho.box] _autodealoc
    cdef morpho.box* ptr(self):
        return < morpho.box * > self._ptr

    @property
    def min_corner(self):
        return Point.from_value(self.ptr().min_corner())

    @property
    def max_corner(self):
        return Point.from_value(self.ptr().max_corner())

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


# ----------------------------------------------------------------------------------------------------------------------
cdef class Linestring(_ArrayT):
    cdef std.unique_ptr[morpho.linestring] _autodealoc
    cdef morpho.linestring * ptr(self):
        return < morpho.linestring *> self._ptr

    @staticmethod
    cdef Linestring from_ptr(morpho.linestring *ptr, bool owner=False):
        cdef Linestring obj = Linestring.__new__(Linestring)
        obj._ptr = ptr
        #Create np array
        cdef np.npy_intp size[2]
        size[0] = ptr.size()
        size[1] = 3
        obj.nparray = np.PyArray_SimpleNewFromData(2, size, np.NPY_DOUBLE, ptr.data())
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef Linestring from_value(const morpho.linestring &ref):
        cdef morpho.linestring* ptr = new morpho.linestring(ref)
        return Linestring.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Circle(_py__base):
    cdef std.unique_ptr[morpho.circle] _autodealoc
    cdef morpho.circle * ptr(self):
        return < morpho.circle *> self._ptr

    @property
    def center(self):
        cdef morpho.point p = self.ptr().get_center()
        cdef const double* pts = p.data()
        return [pts[0], pts[1], pts[2]]

    @property
    def radius(self):
        return self.ptr().get_radius()

    @staticmethod
    cdef Circle from_ptr(morpho.circle *ptr, bool owner=False):
        cdef Circle obj = Circle.__new__(Circle)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef Circle from_ref(const morpho.circle &ref):
        return Circle.from_ptr(<morpho.circle*>&ref)

    @staticmethod
    cdef Circle from_value(const morpho.circle &ref):
        cdef morpho.circle* ptr = new morpho.circle(ref)
        return Circle.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Cone(_py__base):
    cdef std.unique_ptr[morpho.cone] _autodealoc
    cdef morpho.cone * ptr(self):
        return <morpho.cone *> self._ptr

    @property
    def center(self):
        cdef morpho.point p = self.ptr().get_center[zero_t]()
        cdef const double* pts = p.data()
        return [pts[0], pts[1], pts[2]]

    @property
    def radius(self):
        return self.ptr().get_radius[zero_t]()

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
# Circle pipe
#  - This class is somewhat special in the sense that it represents a Vector of circles
#  - However circles are not stack-alloc'able in Cyhton (no default ctor) so we cant iterate
# over them directly to create a list
#  - Therefore we implemented array and iterator interface by getting the elements from their address and wrapping
# in their corresponding Python class, which is executed the exact moment the object is required.
#  - By using this technique, the circle_pipe can be passed on to another function without even incurring
# wrapping/unwrapping of its elements
# ----------------------------------------------------------------------------------------------------------------------
cdef class CirclePipe(_py__base):
    cdef std.unique_ptr[morpho.circle_pipe] _autodealoc
    cdef morpho.circle_pipe * ptr(self):
        return < morpho.circle_pipe *> self._ptr

    # Pass on the array API
    def __getitem__(self, int index):
        if index >= len(self) or index < 0:
            raise IndexError("Circle pipe object is %d circles long. Requested:%d"%(len(self), index))
        cdef std.vector[morpho.circle] *cp = <std.vector[morpho.circle]*>self._ptr
        cdef morpho.circle * mcircle = cp.data()
        # This circle doesnt own C data, we lend him memory from the vector
        return Circle.from_ptr( &mcircle[index] )

    def __iter__(self):
        cdef int i
        #Generator is also iterator
        return (self[i] for i in range(len(self)))

    def __len__(self):
        return self.ptr().size()

    def size(self):
        print("Info: the current object implements iterator and array interface. use len(obj) instead of .size()")
        return len(self)

    @staticmethod
    cdef CirclePipe from_ptr(morpho.circle_pipe *ptr, bool owner=False):
        cdef CirclePipe obj = CirclePipe.__new__(CirclePipe)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        return obj

    @staticmethod
    cdef CirclePipe from_value(const morpho.circle_pipe &ref):
        cdef morpho.circle_pipe* ptr = new morpho.circle_pipe(ref)
        return CirclePipe.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class PointVector(_py__base):
    cdef std.vector[morpho.point] * ptr(self):
        return <std.vector[morpho.point] *> self._ptr

    # Pass on the array API
    def __getitem__(self, int index):
        if index >= len(self) or index < 0:
            raise IndexError("Length is %d. Requested:%d"%(len(self), index))
        cdef morpho.point * point0 = self.ptr().data()
        # This point doesnt own C data, we lend him memory from the vector
        return Point.from_ptr( &point0[index] )

    def __iter__(self):
        cdef int i
        #Generator is also iterator
        return (self[i] for i in range(len(self)))

    def __len__(self):
        return self.ptr().size()

    def size(self):
        print("Info: the current object implements iterator and array interface. use len(obj) instead of .size()")
        return len(self)

    @staticmethod
    cdef PointVector from_ptr(const std.vector[morpho.point] *ptr):
        cdef PointVector obj = PointVector.__new__(PointVector)
        obj._ptr = <std.vector[morpho.point] *>ptr
        return obj

    @staticmethod
    cdef PointVector from_ref(const std.vector[morpho.point] &ref):
        return PointVector.from_ptr(&ref)


# ----------------------------------------------------------------------------------------------------------------------
# Main Data Structures
# ----------------------------------------------------------------------------------------------------------------------
cdef class Mat_Points(_ArrayT):
    cdef std.unique_ptr[morpho.mat_points] _autodealoc
    cdef morpho.mat_points * ptr(self):
        return <morpho.mat_points *> self._ptr

    @staticmethod
    cdef Mat_Points from_ptr(morpho.mat_points * matpoints, bool owner=False):
        cdef Mat_Points obj = Mat_Points()
        obj._ptr = matpoints
        if owner: obj._autodealoc.reset(matpoints)

        # Create a numpy array (memviews dont expose no nicely to python)
        cdef np.npy_intp[2] dim
        dim[0] = matpoints.size1()
        dim[1] = matpoints.size2()
        obj.nparray = np.PyArray_SimpleNewFromData(2, dim, np.NPY_DOUBLE, matpoints.data().begin())
        return obj

    @staticmethod
    cdef Mat_Points from_ref(const morpho.mat_points &ref):
        return Mat_Points.from_ptr(<morpho.mat_points*>&ref)

    @staticmethod
    cdef Mat_Points from_value(const morpho.mat_points &ref):
        cdef morpho.mat_points* ptr = new morpho.mat_points(ref)
        return Mat_Points.from_ptr(ptr, True)


# ----------------------------------------------------------------------------------------------------------------------
cdef class Mat_Index(_ArrayT):
    cdef std.unique_ptr[morpho.mat_index ] _autodealoc
    cdef morpho.mat_index* ptr(self):
        return <morpho.mat_index *> self._ptr

    @staticmethod
    cdef Mat_Index from_ptr(morpho.mat_index *ptr, bool owner=False):
        cdef Mat_Index obj = Mat_Index.__new__(Mat_Index)
        obj._ptr = ptr
        if owner: obj._autodealoc.reset(ptr)
        # Create a numpy array
        cdef np.npy_intp[2] dim
        dim[0] = ptr.size1()
        dim[1] = ptr.size2()
        obj.nparray = np.PyArray_SimpleNewFromData(2, dim, np.NPY_INT, ptr.data().begin())
        return obj

    @staticmethod
    cdef Mat_Index from_ref(const morpho.mat_index &ref):
        return Mat_Index.from_ptr(<morpho.mat_index*>&ref)

    @staticmethod
    cdef Mat_Index from_value(const morpho.mat_index &ref):
        cdef morpho.mat_index *ptr = new morpho.mat_index(ref)
        return Mat_Index.from_ptr(ptr, True)