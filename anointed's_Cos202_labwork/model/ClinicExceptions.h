#ifndef CLINICEXCEPTIONS_H
#define CLINICEXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace clinic_mgmt {

class DuplicateIdException : public std::runtime_error {
public:
    DuplicateIdException(const std::string& id)
        : std::runtime_error("ID already registered: " + id) {}
};

class InvalidInputException : public std::runtime_error {
public:
    InvalidInputException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class FileErrorException : public std::runtime_error {
public:
    FileErrorException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class RecordNotFoundException : public std::runtime_error {
public:
    RecordNotFoundException(const std::string& msg)
        : std::runtime_error(msg) {}
};

}

#endif
