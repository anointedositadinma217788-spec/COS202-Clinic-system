#ifndef CLINICEXCEPTIONS_H
#define CLINICEXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace model {

// custom exceptions, every one carries a short title for the error dialog
class ClinicException : public std::runtime_error {
private:
    std::string title;

public:
    ClinicException(const std::string& title, const std::string& message)
        : std::runtime_error(message), title(title) {}
    virtual ~ClinicException() throw() {}

    std::string getTitle() const { return title; }
};

class InvalidInputException : public ClinicException {
public:
    InvalidInputException(const std::string& message)
        : ClinicException("Invalid Patient Information", message) {}
};

class InvalidAgeException : public ClinicException {
public:
    InvalidAgeException()
        : ClinicException("Invalid Age", "Patient age must be between 0 and 120.") {}
};

class DuplicateIdException : public ClinicException {
public:
    DuplicateIdException(const std::string& id)
        : ClinicException("Duplicate ID", "Patient ID " + id + " already exists.") {}
};

class InvalidAppointmentException : public ClinicException {
public:
    InvalidAppointmentException(const std::string& message)
        : ClinicException("Invalid Appointment", message) {}
};

class RecordNotFoundException : public ClinicException {
public:
    RecordNotFoundException(const std::string& message)
        : ClinicException("Record Not Found", message) {}
};

class FileErrorException : public ClinicException {
public:
    FileErrorException(const std::string& message)
        : ClinicException("File Error", message) {}
};

class InvalidSearchException : public ClinicException {
public:
    InvalidSearchException(const std::string& message)
        : ClinicException("Invalid Search", message) {}
};

}

#endif
