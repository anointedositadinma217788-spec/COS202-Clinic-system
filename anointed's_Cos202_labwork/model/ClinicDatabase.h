#ifndef CLINICDATABASE_H
#define CLINICDATABASE_H

#include "PatientRecord.h"
#include "Appointment.h"
#include <vector>
#include <string>

namespace model {

// owns every patient and appointment object in the program
class ClinicDatabase {
private:
    std::vector<PatientRecord*> patients;
    std::vector<Appointment*> appointments;

    // fixed size array used as a cache of recently opened patients
    static const int CACHE_SIZE = 10;
    PatientRecord* recentPatients[CACHE_SIZE];
    int recentCount;

public:
    ClinicDatabase();
    ~ClinicDatabase();

    void addPatient(PatientRecord* patient);
    bool deletePatient(const std::string& patientId);
    PatientRecord* findPatient(const std::string& patientId) const;
    const std::vector<PatientRecord*>& getPatients() const;
    void setPatientOrder(const std::vector<PatientRecord*>& newOrder);

    void addAppointment(Appointment* appt);
    bool removeAppointment(int number);
    Appointment* findAppointment(int number) const;
    const std::vector<Appointment*>& getAppointments() const;

    void clearAll();

    void markAccessed(PatientRecord* patient);
    void removeFromCache(PatientRecord* patient);
    PatientRecord* findInCache(const std::string& patientId) const;
    int getRecentCount() const;
    PatientRecord* getRecent(int index) const;
    int getCacheSize() const;
};

}

#endif
