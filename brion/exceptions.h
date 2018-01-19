namespace brion
{
class Error: protected std::exception
{
protected:
    std::string msg;

public:
Error(const std::string& _msg): msg(_msg) {}
    virtual const char* what() {return msg.c_str();}
};

class RawDataError: public Error {
public:
RawDataError(const std::string& _msg) : Error(_msg) {}
};

class UnknownFileType : public Error {
public:
UnknownFileType(const std::string& _msg) : Error(_msg) {}
};

class SomaError: public Error {
public:
SomaError(const std::string& _msg) : Error(_msg) {}
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

}
