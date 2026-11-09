#include "Appointment.h"
#include <iostream>

namespace clinic_mgmt {

Appointment::Appointment(int num, std::string preg, std::string doc,
                         std::string time, std::string service, std::string status)
    : apptNumber(num), patientReg(preg), doctor(doc), appointmentTime(time),
      serviceType(service), apptStatus(status) {}

Appointment::~Appointment() {}

int Appointment::getApptNumber() const { return apptNumber; }
std::string Appointment::getPatientReg() const { return patientReg; }
std::string Appointment::getDoctor() const { return doctor; }
std::string Appointment::getAppointmentTime() const { return appointmentTime; }
std::string Appointment::getServiceType() const { return serviceType; }
std::string Appointment::getApptStatus() const { return apptStatus; }

void Appointment::setDoctor(std::string doc) { doctor = doc; }
void Appointment::setAppointmentTime(std::string time) { appointmentTime = time; }
void Appointment::setServiceType(std::string service) { serviceType = service; }
void Appointment::setApptStatus(std::string status) { apptStatus = status; }

void Appointment::showDetails() const {
    std::cout << "Appt No: " << apptNumber << std::endl;
    std::cout << "Patient Reg: " << patientReg << std::endl;
    std::cout << "Doctor: " << doctor << std::endl;
    std::cout << "Time: " << appointmentTime << std::endl;
    std::cout << "Service: " << serviceType << std::endl;
    std::cout << "Status: " << apptStatus << std::endl;
}

}
