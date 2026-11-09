#ifndef CLINICDATABASE_H
#define CLINICDATABASE_H

#include "PatientRecord.h"
#include "Appointment.h"
#include <vector>
#include <queue>
#include <stack>
#include <string>

namespace clinic_mgmt {

struct UndoEntry {
    int action;
    std::string patientType;
    std::string reg;
    std::string name;
    int age;
    std::string contact;
    std::string field1;
    std::string field2;
    int apptNum;
};

class ClinicDatabase {
private:
    std::vector<PatientRecord*> patientList;
    std::vector<Appointment*> apptList;
    std::queue<std::string> waitingQueue;
    std::stack<UndoEntry> undoStack;

    static const int MAX_DOCTORS = 8;
    std::string doctorList[MAX_DOCTORS];
    int doctorCount;

    int nextApptNum;

    PatientRecord* recreatePatient(const UndoEntry& entry) const;
    int countCategoryRecursive(size_t index, const std::string& category) const;
    PatientRecord* binarySearchRec(std::vector<PatientRecord*>& list,
                                   long low, long high, const std::string& reg) const;
    void selectionSortByReg(std::vector<PatientRecord*>& list) const;
    void removeAppointmentDirect(int apptNum);

public:
    ClinicDatabase();
    ~ClinicDatabase();

    void addPatient(PatientRecord* patient);
    bool editPatient(std::string reg, std::string newName, int newAge, std::string newContact);
    bool removePatient(std::string reg);
    PatientRecord* findPatientByReg(std::string reg) const;
    std::vector<PatientRecord*> getAllPatients() const;
    std::vector<PatientRecord*> linearSearchByName(std::string name) const;

    PatientRecord* binarySearchByReg(std::string reg);
    void selectionSortByAgePublic(std::vector<PatientRecord*>& list);
    void insertionSortByName(std::vector<PatientRecord*>& list);

    void addAppointment(std::string reg, std::string doctor, std::string time,
                        std::string service);
    bool cancelAppointment(int apptNum);
    std::vector<Appointment*> getAllAppointments() const;
    int getNextApptNum() const { return nextApptNum; }

    void addToQueue(std::string reg);
    bool callNextPatient(std::string& result);
    std::vector<std::string> queueSnapshot() const;
    int queueSize() const { return (int)waitingQueue.size(); }

    bool undoLastAction();

    int doctorCount_count() const { return doctorCount; }
    std::string getDoctor(int index) const;

    int recursiveCountByCategory(const std::string& category) const;

    void saveAll();
    void loadAll();
};

}

#endif
