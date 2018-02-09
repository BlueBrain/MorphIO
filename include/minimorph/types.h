#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <stack>
#include <queue>

#include <stdint.h>

#include <minimorph/vector_types.h>
#include <minimorph/enums.h>
#include <minimorph/exceptions.h>


#ifdef __GNUC__
#define BRAIN_UNUSED __attribute__((unused))
#else
#define BRAIN_UNUSED
#endif

// TODO: bcoste fix me
#include<iostream>
#define LBTHROW(x) (throw x)
#define LBERROR(x) (std::cerr << x << std::endl)
#define LBWARN std::cerr

/** @namespace minimorph Blue Brain File IO classes */
namespace minimorph
{
typedef std::string URI;

using namespace enums;
class Morphology;
class MorphologyInitData;
class Section;
class Soma;

namespace Property {
struct Properties;
}


template <typename T> class Iterator;

typedef std::shared_ptr<Property::Properties> PropertiesPtr;

typedef std::pair<size_t, size_t> SectionRange;

typedef Iterator<std::stack<Section>> depth_iterator;
typedef Iterator<std::queue<Section>> breadth_iterator;
typedef Iterator<std::shared_ptr<Section>> upstream_iterator;


typedef std::vector<size_t> size_ts;
typedef std::vector<int32_t> int32_ts;
typedef std::vector<uint16_t> uint16_ts;
typedef std::vector<uint32_t> uint32_ts;
typedef std::vector<uint64_t> uint64_ts;
typedef std::vector<float> floats;
typedef std::vector<double> doubles;
typedef std::vector<Vector2i> Vector2is;
typedef std::vector<Vector3f> Vector3fs;
typedef std::vector<Vector4f> Vector4fs;
typedef std::vector<Vector3d> Vector3ds;
typedef std::vector<Vector4d> Vector4ds;
typedef std::vector<SectionType> SectionTypes;
typedef std::vector<Section> Sections;

typedef std::shared_ptr<int32_ts> int32_tsPtr;
typedef std::shared_ptr<uint16_ts> uint16_tsPtr;
typedef std::shared_ptr<uint32_ts> uint32_tsPtr;
typedef std::shared_ptr<floats> floatsPtr;
typedef std::shared_ptr<doubles> doublesPtr;
typedef std::shared_ptr<Vector3fs> Vector3fsPtr;
typedef std::shared_ptr<Vector3ds> Vector3dsPtr;
typedef std::shared_ptr<Vector4ds> Vector4dsPtr;

using MorphologyPtr = std::shared_ptr<Morphology>;
using ConstMorphologyPtr = std::shared_ptr<const Morphology>;

typedef std::vector<std::string> Strings;

}

//TODO: compile
#if 0
// if you have a type T in namespace N, the operator << for T needs to be in
// namespace N too
namespace boost
{
template <typename T>
inline std::ostream& operator<<(std::ostream& os,
                                const boost::multi_array<T, 2>& data)
{
    for (size_t i = 0; i < data.shape()[0]; ++i)
    {
        for (size_t j = 0; j < data.shape()[1]; ++j)
            os << data[i][j] << " ";
        os << std::endl;
    }
    return os;
}
}

namespace std
{
template <class T, class U>
inline std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& pair)
{
    return os << "[ " << pair.first << ", " << pair.second << " ]";
}
}
#endif
