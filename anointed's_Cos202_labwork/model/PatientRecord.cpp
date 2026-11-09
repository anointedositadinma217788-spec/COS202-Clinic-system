#include "PatientRecord.h"
#include "Appointment.h"
#include "../utils/Validator.h"
#include <sstream>

namespace model {

PatientRecord::PatientRecord(const std::string& patientId, const std::string& name, int age,
                             const std::string& contact)
    : patientId(patientId), name(name), age(age), contact(contact) {}

PatientRecord::~PatientRecord() {}

std::string PatientRecord::getId() const { return patientId; }
std::string PatientRecord::getName() const { return name; }
int PatientRecord::getAge() const { return age; }
std::string PatientRecord::getContact() const { return contact; }

void PatientRecord::setName(const std::string& newName) { name = newName; }
void PatientRecord::setAge(int newAge) { age = newAge; }
void PatientRecord::setContact(const std::string& newContact) { contact = newContact; }

std::string PatientRecord::getDetails() const {
    std::stringstream out;
    out << "Patient ID: " << patientId << "\n";
    out << "Name: " << name << "\n";
    out << "Age: " << age << "\n";
    out << "Contact: " << contact << "\n";
    out << "Category: " << getCategory() << " (" << getPriorityLabel() << ")\n";
    out << "Appointments: " << appointments.size() << " total, "
        << countAppointments("Scheduled") << " pending, "
        << countAppointments("Completed") << " completed\n";
    out << "Consultation fee: N" << getConsultationFee() << "\n";
    return out.str();
}

// general patients have no extra fields, subclasses override these
std::string PatientRecord::getExtraInfo() const { return "-"; }
std::string PatientRecord::getExtra1() const { return ""; }
std::string PatientRecord::getExtra2() const { return ""; }
void PatientRecord::setExtraFields(const std::string&, const std::string&) {}

// Appointable part
void PatientRecord::scheduleAppointment(Appointment* appt) {
    if (appt != NULL) {
        appointments.push_back(appt);
    }
}

bool PatientRecord::cancelAppointment(int apptNo) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i]->getNumber() == apptNo) {
            appointments[i]->setStatus("Cancelled");
            return true;
        }
    }
    return false;
}

std::vector<Appointment*> PatientRecord::getAppointments() const {
    return appointments;
}

void PatientRecord::unlinkAppointment(int apptNo) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i]->getNumber() == apptNo) {
            appointments.erase(appointments.begin() + i);
            return;
        }
    }
}

int PatientRecord::countAppointments(const std::string& status) const {
    int count = 0;
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i]->getStatus() == status) {
            count++;
        }
    }
    return count;
}

// earliest scheduled appointment, empty if there is none
std::string PatientRecord::getNextAppointment() const {
    std::string earliest = "";
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i]->isPending()) {
            std::string when = appointments[i]->getDateTime();
            if (earliest.empty() || when < earliest) {
                earliest = when;
            }
        }
    }
    return earliest;
}

}
