#pragma once
#include <stdexcept>

namespace morphio {
/**
   Base class of all morphio errors
 **/
class MorphioError : public std::runtime_error
{
public:
    MorphioError(const std::string& _msg)
        : std::runtime_error(_msg)
    {
    }
};

class NotImplementedError : public MorphioError
{
public:
    NotImplementedError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
};

class RawDataError : public MorphioError
{
public:
    RawDataError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
};

class UnknownFileType : public MorphioError
{
public:
    UnknownFileType(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
};

class SomaError : public MorphioError
{
public:
    SomaError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
};

class IDSequenceError : public RawDataError
{
public:
    IDSequenceError(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
};

class MultipleTrees : public RawDataError
{
public:
    MultipleTrees(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
};

class MissingParentError : public RawDataError
{
public:
    MissingParentError(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
};

class SectionBuilderError : public RawDataError
{
public:
    SectionBuilderError(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
};

class WriterError : public MorphioError
{
public:
    WriterError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
};
} // namespace morphio
