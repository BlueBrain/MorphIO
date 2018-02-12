#pragma once

namespace minimorph
{

class RawDataError: public std::runtime_error {
public:
RawDataError(const std::string& _msg) : std::runtime_error(_msg) {}
};

class UnknownFileType : public std::runtime_error {
public:
UnknownFileType(const std::string& _msg) : std::runtime_error(_msg) {}
};

class SomaError: public std::runtime_error {
public:
SomaError(const std::string& _msg) : std::runtime_error(_msg) {}
};

class IDSequenceError: public RawDataError {
public:
IDSequenceError(const std::string& _msg) : RawDataError(_msg) {}
};

class MultipleTrees: public RawDataError {
public:
MultipleTrees(const std::string& _msg) : RawDataError(_msg) {}
};

class MissingParentError: public RawDataError {
public:
MissingParentError(const std::string& _msg) : RawDataError(_msg) {}
};

class SectionBuilderError: public RawDataError {
public:
SectionBuilderError(const std::string& _msg) : RawDataError(_msg) {}
};


}
