#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>

namespace model {

class Appointment {
private:
    int number;
    std::string patientId;
    std::string doctor;
    std::string date;
    std::string time;
    std::string service;
    std::string status;

public:
    Appointment(int number, const std::string& patientId, const std::string& doctor,
                const std::string& date, const std::string& time,
                const std::string& service, const std::string& status);

    int getNumber() const;
    std::string getPatientId() const;
    std::string getDoctor() const;
    std::string getDate() const;
    std::string getTime() const;
    std::string getService() const;
    std::string getStatus() const;
    std::string getDateTime() const;
    bool isPending() const;

    void setStatus(const std::string& newStatus);
};

}

#endif
