#include "Appointment.h"

namespace model {

Appointment::Appointment(int number, const std::string& patientId, const std::string& doctor,
                         const std::string& date, const std::string& time,
                         const std::string& service, const std::string& status)
    : number(number), patientId(patientId), doctor(doctor), date(date), time(time),
      service(service), status(status) {}

int Appointment::getNumber() const { return number; }
std::string Appointment::getPatientId() const { return patientId; }
std::string Appointment::getDoctor() const { return doctor; }
std::string Appointment::getDate() const { return date; }
std::string Appointment::getTime() const { return time; }
std::string Appointment::getService() const { return service; }
std::string Appointment::getStatus() const { return status; }

std::string Appointment::getDateTime() const {
    return date + " " + time;
}

bool Appointment::isPending() const {
    return status == "Scheduled";
}

void Appointment::setStatus(const std::string& newStatus) {
    status = newStatus;
}

}
