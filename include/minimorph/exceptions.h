#pragma once

namespace minimorph
{
/**
   Base class of all minimorph errors
 **/
class MinimorphError: public std::runtime_error {
public:
    MinimorphError(const std::string& _msg) : std::runtime_error(_msg) {}
};

class RawDataError: public MinimorphError {
public:
    RawDataError(const std::string& _msg) : MinimorphError(_msg) {}
};

class UnknownFileType : public MinimorphError {
public:
UnknownFileType(const std::string& _msg) : MinimorphError(_msg) {}
};

class SomaError: public MinimorphError {
public:
SomaError(const std::string& _msg) : MinimorphError(_msg) {}
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
}
