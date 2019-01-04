
#ifndef BRAIN_DETAIL_LOCKHDF5
#define BRAIN_DETAIL_LOCKHDF5

#include <mutex>

namespace morphio {
namespace detail {
// Every access to hdf5 must be serialized if HDF5 does not take care of it
// which needs a thread-safe built of the library.
// http://www.hdfgroup.org/hdf5-quest.html#gconc
inline std::mutex* hdf5Lock()
{
    static std::mutex _hdf5Lock;
    return &_hdf5Lock;
}
} // namespace detail
} // namespace morphio

#endif
