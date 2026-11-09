#ifndef CLINICMANAGER_H
#define CLINICMANAGER_H

#include "UndoAction.h"
#include "AppointmentController.h"
#include "SearchEngine.h"
#include "SortEngine.h"
#include "../model/ClinicDatabase.h"
#include "../utils/IDGenerator.h"
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include <ctime>

namespace controller {

struct QueueEntry {
    std::string patientId;
    time_t joinedAt;
};

// main business logic, the GUI talks to the clinic only through this class
class ClinicManager {
private:
    model::ClinicDatabase database;
    std::queue<QueueEntry> waitingQueue;
    std::stack<UndoAction> undoStack;
    utils::IDGenerator idGenerator;
    AppointmentController appointmentController;
    SearchEngine searchEngine;
    SortEngine sortEngine;

    int sortedBy;
    int sortedWith;
    bool unsavedChanges;
    std::string dataFolder;

    void checkPatientInput(const std::string& patientId, const std::string& name,
                           const std::string& ageText, const std::string& contact, int& age) const;
    void removeFromQueue(const std::string& patientId);

public:
    ClinicManager();

    void setDataFolder(const std::string& folder);
    std::string getDataFolder() const;

    // patients
    std::string suggestPatientId() const;
    void registerPatient(const std::string& category, const std::string& patientId,
                         const std::string& name, const std::string& ageText,
                         const std::string& contact, const std::string& extra1,
                         const std::string& extra2);
    void editPatient(const std::string& category, const std::string& patientId,
                     const std::string& name, const std::string& ageText,
                     const std::string& contact, const std::string& extra1,
                     const std::string& extra2);
    void deletePatient(const std::string& patientId);
    model::PatientRecord* openPatient(const std::string& patientId);
    model::PatientRecord* peekPatient(const std::string& patientId) const;
    const std::vector<model::PatientRecord*>& getPatients() const;
    std::string describePatient(const model::PatientRecord& patient) const;
    const model::ClinicDatabase& getDatabase() const;

    // appointments
    int scheduleAppointment(const std::string& patientId, const std::string& doctor,
                            const std::string& date, const std::string& time,
                            const std::string& service);
    void cancelAppointment(int number);
    void completeAppointment(int number);
    void removeAppointment(int number);
    const std::vector<model::Appointment*>& getAppointments() const;
    std::vector<model::Appointment*> getAppointmentsOn(const std::string& date) const;
    std::vector<model::Appointment*> getDueSoon(int minutesAhead) const;
    int getDoctorCount() const;
    std::string getDoctor(int index) const;

    // search and sort
    std::vector<model::PatientRecord*> searchPatients(int field, int method, const std::string& keyword,
                                                      std::string& methodUsed);
    void sortPatients(int algorithm, int field);
    std::string getSortState() const;
    std::string describeSearch(int field, int method) const;
    std::string explainSort(int algorithm) const;

    // waiting queue
    void addToQueue(const std::string& patientId);
    bool callNextPatient(QueueEntry& served);
    std::vector<QueueEntry> getQueue() const;
    int getQueueSize() const;

    // undo
    bool undoLast(std::string& whatWasUndone);
    int getUndoCount() const;
    std::vector<std::string> getUndoHistory() const;

    // files
    void saveData();
    int loadData();
    void newData();
    void exportPatients(const std::string& path) const;
    int importPatients(const std::string& path, int& skipped);
    bool hasUnsavedChanges() const;
};

}

#endif
