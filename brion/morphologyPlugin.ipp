/* Copyright (c) 2017, EPFL/Blue Brain Project
 *                          Stefan.Eilemann@epfl.ch
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

namespace brion
{
namespace
{
template <typename T>
size_t _getSerializationSize(const std::vector<T>& vector)
{
    return sizeof(uint64_t) + vector.size() * sizeof(T);
}

template <typename T>
void _serializeArray(uint8_t*& dst, const std::vector<T>& src)
{
    const uint64_t arraySize = src.size();
    *reinterpret_cast<uint64_t*>(dst) = arraySize;
    dst += sizeof(uint64_t);

    if (arraySize > 0)
    {
        memcpy(dst, src.data(), sizeof(T) * src.size());
        dst += sizeof(T) * src.size();
    }
}

template <typename T>
bool _deserializeArray(std::vector<T>& dst, const uint8_t*& src,
                       const uint8_t* end)
{
    if (src + sizeof(uint64_t) > end)
        return false;
    const uint64_t arraySize = *reinterpret_cast<const uint64_t*>(src);
    src += sizeof(uint64_t);

    if (arraySize == 0)
    {
        dst.clear();
        return true;
    }

    if (src + sizeof(T) * arraySize > end)
        return false;
    const T* srcPtr = reinterpret_cast<const T*>(src);
    dst.assign(srcPtr, srcPtr + arraySize);
    src += sizeof(T) * arraySize;
    return true;
}
}

servus::Serializable::Data inline MorphologyPlugin::_toBinary() const
{
    servus::Serializable::Data data;
    data.size = sizeof(MorphologyVersion) + sizeof(CellFamily) +
                _getSerializationSize(_points) +
                _getSerializationSize(_sections) +
                _getSerializationSize(_sectionTypes) +
                _getSerializationSize(_perimeters);

    uint8_t* ptr = new uint8_t[data.size];
    data.ptr.reset(ptr, std::default_delete<uint8_t[]>());

    *reinterpret_cast<MorphologyVersion*>(ptr) = _data.version;
    ptr += sizeof(MorphologyVersion);

    *reinterpret_cast<CellFamily*>(ptr) = _data.family;
    ptr += sizeof(CellFamily);

    _serializeArray(ptr, _points);
    _serializeArray(ptr, _sections);
    _serializeArray(ptr, _sectionTypes);
    _serializeArray(ptr, _perimeters);
    return data;
}

bool inline MorphologyPlugin::_fromBinary(const void* data, const size_t size)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* const end = ptr + size;
    if (size < sizeof(MorphologyVersion) + sizeof(CellFamily))
        return false;

    _data.version = *reinterpret_cast<const MorphologyVersion*>(ptr);
    ptr += sizeof(MorphologyVersion);

    _data.family = *reinterpret_cast<const CellFamily*>(ptr);
    ptr += sizeof(CellFamily);

    if (_deserializeArray(_points, ptr, end) &&
        _deserializeArray(_sections, ptr, end) &&
        _deserializeArray(_sectionTypes, ptr, end) &&
        _deserializeArray(_perimeters, ptr, end))
    {
        return true;
    }

    _points.clear();
    _sections.clear();
    _sectionTypes.clear();
    _perimeters.clear();
    return false;
}
}
