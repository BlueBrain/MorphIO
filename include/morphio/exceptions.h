#pragma once

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
    virtual ~MorphioError() {}
};

class NotImplementedError : public MorphioError
{
public:
    NotImplementedError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
    virtual ~NotImplementedError() {}
};

class RawDataError : public MorphioError
{
public:
    RawDataError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
    virtual ~RawDataError() {}
};

class UnknownFileType : public MorphioError
{
public:
    UnknownFileType(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
    virtual ~UnknownFileType() {}
};

class SomaError : public MorphioError
{
public:
    SomaError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
    virtual ~SomaError() {}
};

class IDSequenceError : public RawDataError
{
public:
    IDSequenceError(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
    virtual ~IDSequenceError() {}
};

class MultipleTrees : public RawDataError
{
public:
    MultipleTrees(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
    virtual ~MultipleTrees() {}
};

class MissingParentError : public RawDataError
{
public:
    MissingParentError(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
    virtual ~MissingParentError() {}
};

class SectionBuilderError : public RawDataError
{
public:
    SectionBuilderError(const std::string& _msg)
        : RawDataError(_msg)
    {
    }
    virtual ~SectionBuilderError() {}
};

class WriterError : public MorphioError
{
public:
    WriterError(const std::string& _msg)
        : MorphioError(_msg)
    {
    }
    virtual ~WriterError() {}
};
} // namespace morphio
