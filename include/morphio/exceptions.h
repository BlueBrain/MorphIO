/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <stdexcept>  // std::runtime_error

namespace morphio {

/** Base class of all morphio errors */
class MorphioError: public std::runtime_error
{
  public:
    explicit MorphioError(const std::string& _msg)
        : std::runtime_error(_msg) {}
};

class NotImplementedError: public MorphioError
{
  public:
    explicit NotImplementedError(const std::string& _msg)
        : MorphioError(_msg) {}
};

class RawDataError: public MorphioError
{
  public:
    explicit RawDataError(const std::string& _msg)
        : MorphioError(_msg) {}
};

class UnknownFileType: public MorphioError
{
  public:
    explicit UnknownFileType(const std::string& _msg)
        : MorphioError(_msg) {}
};

class SomaError: public MorphioError
{
  public:
    explicit SomaError(const std::string& _msg)
        : MorphioError(_msg) {}
};

class IDSequenceError: public RawDataError
{
  public:
    explicit IDSequenceError(const std::string& _msg)
        : RawDataError(_msg) {}
};

class MultipleTrees: public RawDataError
{
  public:
    explicit MultipleTrees(const std::string& _msg)
        : RawDataError(_msg) {}
};

class MissingParentError: public RawDataError
{
  public:
    explicit MissingParentError(const std::string& _msg)
        : RawDataError(_msg) {}
};

class SectionBuilderError: public RawDataError
{
  public:
    explicit SectionBuilderError(const std::string& _msg)
        : RawDataError(_msg) {}
};

class WriterError: public MorphioError
{
  public:
    explicit WriterError(const std::string& _msg)
        : MorphioError(_msg) {}
};
}  // namespace morphio
