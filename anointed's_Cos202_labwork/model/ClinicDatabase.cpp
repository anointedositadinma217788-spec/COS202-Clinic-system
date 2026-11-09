#include "ClinicDatabase.h"
#include "ClinicExceptions.h"

namespace model {

ClinicDatabase::ClinicDatabase() {
    recentCount = 0;
    for (int i = 0; i < CACHE_SIZE; i++) {
        recentPatients[i] = NULL;
    }
}

ClinicDatabase::~ClinicDatabase() {
    clearAll();
}

void ClinicDatabase::addPatient(PatientRecord* patient) {
    if (findPatient(patient->getId()) != NULL) {
        throw DuplicateIdException(patient->getId());
    }
    patients.push_back(patient);
}

// removes the patient together with all of their appointments
bool ClinicDatabase::deletePatient(const std::string& patientId) {
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i]->getId() == patientId) {
            PatientRecord* patient = patients[i];
            for (int j = (int)appointments.size() - 1; j >= 0; j--) {
                if (appointments[j]->getPatientId() == patientId) {
                    delete appointments[j];
                    appointments.erase(appointments.begin() + j);
                }
            }
            removeFromCache(patient);
            delete patient;
            patients.erase(patients.begin() + i);
            return true;
        }
    }
    return false;
}

PatientRecord* ClinicDatabase::findPatient(const std::string& patientId) const {
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i]->getId() == patientId) {
            return patients[i];
        }
    }
    return NULL;
}

const std::vector<PatientRecord*>& ClinicDatabase::getPatients() const {
    return patients;
}

// used after sorting so the main list keeps the new order
void ClinicDatabase::setPatientOrder(const std::vector<PatientRecord*>& newOrder) {
    if (newOrder.size() == patients.size()) {
        patients = newOrder;
    }
}

void ClinicDatabase::addAppointment(Appointment* appt) {
    appointments.push_back(appt);
    Appointable* owner = findPatient(appt->getPatientId());
    if (owner != NULL) {
        owner->scheduleAppointment(appt);
    }
}

bool ClinicDatabase::removeAppointment(int number) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i]->getNumber() == number) {
            PatientRecord* owner = findPatient(appointments[i]->getPatientId());
            if (owner != NULL) {
                owner->unlinkAppointment(number);
            }
            delete appointments[i];
            appointments.erase(appointments.begin() + i);
            return true;
        }
    }
    return false;
}

Appointment* ClinicDatabase::findAppointment(int number) const {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i]->getNumber() == number) {
            return appointments[i];
        }
    }
    return NULL;
}

const std::vector<Appointment*>& ClinicDatabase::getAppointments() const {
    return appointments;
}

void ClinicDatabase::clearAll() {
    for (size_t i = 0; i < patients.size(); i++) {
        delete patients[i];
    }
    for (size_t i = 0; i < appointments.size(); i++) {
        delete appointments[i];
    }
    patients.clear();
    appointments.clear();
    recentCount = 0;
    for (int i = 0; i < CACHE_SIZE; i++) {
        recentPatients[i] = NULL;
    }
}

// cache: newest at index 0, the oldest one falls off the end when full
void ClinicDatabase::markAccessed(PatientRecord* patient) {
    if (patient == NULL) {
        return;
    }
    int position = -1;
    for (int i = 0; i < recentCount; i++) {
        if (recentPatients[i] == patient) {
            position = i;
        }
    }
    if (position == -1) {
        if (recentCount < CACHE_SIZE) {
            recentCount++;
        }
        position = recentCount - 1;
    }
    for (int i = position; i > 0; i--) {
        recentPatients[i] = recentPatients[i - 1];
    }
    recentPatients[0] = patient;
}

void ClinicDatabase::removeFromCache(PatientRecord* patient) {
    for (int i = 0; i < recentCount; i++) {
        if (recentPatients[i] == patient) {
            for (int j = i; j < recentCount - 1; j++) {
                recentPatients[j] = recentPatients[j + 1];
            }
            recentCount--;
            recentPatients[recentCount] = NULL;
            return;
        }
    }
}

PatientRecord* ClinicDatabase::findInCache(const std::string& patientId) const {
    for (int i = 0; i < recentCount; i++) {
        if (recentPatients[i]->getId() == patientId) {
            return recentPatients[i];
        }
    }
    return NULL;
}

int ClinicDatabase::getRecentCount() const {
    return recentCount;
}

PatientRecord* ClinicDatabase::getRecent(int index) const {
    if (index < 0 || index >= recentCount) {
        return NULL;
    }
    return recentPatients[index];
}

int ClinicDatabase::getCacheSize() const {
    return CACHE_SIZE;
}

}
