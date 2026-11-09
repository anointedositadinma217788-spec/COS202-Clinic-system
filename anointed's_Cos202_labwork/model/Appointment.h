#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>

namespace clinic_mgmt {

class Appointment {
private:
    int apptNumber;
    std::string patientReg;
    std::string doctor;
    std::string appointmentTime;
    std::string serviceType;
    std::string apptStatus;

public:
    Appointment(int num, std::string preg, std::string doc,
                std::string time, std::string service, std::string status = "Scheduled");
    ~Appointment();

    int getApptNumber() const;
    std::string getPatientReg() const;
    std::string getDoctor() const;
    std::string getAppointmentTime() const;
    std::string getServiceType() const;
    std::string getApptStatus() const;

    void setDoctor(std::string doc);
    void setAppointmentTime(std::string time);
    void setServiceType(std::string service);
    void setApptStatus(std::string status);

    void showDetails() const;
};

}

#endif
