#include "ClinicManager.h"
#include "../model/ClinicExceptions.h"
#include "../model/PatientFactory.h"
#include "../utils/Validator.h"
#include "../utils/FileHandler.h"
#include <cctype>
#include <cstdlib>

namespace controller {

ClinicManager::ClinicManager()
    : appointmentController(database, undoStack, idGenerator) {
    sortedBy = -1;
    sortedWith = -1;
    unsavedChanges = false;
    dataFolder = "data";
}

void ClinicManager::setDataFolder(const std::string& folder) {
    dataFolder = folder;
}

std::string ClinicManager::getDataFolder() const {
    return dataFolder;
}

// ---------- patients ----------

void ClinicManager::checkPatientInput(const std::string& patientId, const std::string& name,
                                      const std::string& ageText, const std::string& contact,
                                      int& age) const {
    if (patientId.empty()) {
        throw model::InvalidInputException("Patient ID is required.");
    }
    if (patientId.size() > 10) {
        throw model::InvalidInputException("Patient ID must be 10 characters or less.");
    }
    for (size_t i = 0; i < patientId.size(); i++) {
        if (!std::isalnum((unsigned char)patientId[i])) {
            throw model::InvalidInputException("Patient ID can only contain letters and numbers.");
        }
    }
    if (name.empty()) {
        throw model::InvalidInputException("Patient name is required.");
    }
    for (size_t i = 0; i < name.size(); i++) {
        char c = name[i];
        if (!std::isalpha((unsigned char)c) && c != ' ' && c != '-' && c != '.' && c != '\'') {
            throw model::InvalidInputException("Patient name can only contain letters, spaces, hyphens and full stops.");
        }
    }
    if (!utils::isWholeNumber(ageText) || ageText.size() > 3) {
        throw model::InvalidAgeException();
    }
    age = std::atoi(ageText.c_str());
    if (age < 0 || age > 120) {
        throw model::InvalidAgeException();
    }
    int digits = 0;
    for (size_t i = 0; i < contact.size(); i++) {
        char c = contact[i];
        if (std::isdigit((unsigned char)c)) {
            digits++;
        } else if (c != '+' && c != ' ' && c != '-') {
            throw model::InvalidInputException("Contact must be a phone number (digits, spaces, + or -).");
        }
    }
    if (digits < 7 || digits > 15) {
        throw model::InvalidInputException("Contact phone number must have between 7 and 15 digits.");
    }
}

std::string ClinicManager::suggestPatientId() const {
    return idGenerator.nextPatientId();
}

void ClinicManager::registerPatient(const std::string& category, const std::string& patientId,
                                    const std::string& name, const std::string& ageText,
                                    const std::string& contact, const std::string& extra1,
                                    const std::string& extra2) {
    std::string id = utils::toUpper(utils::trim(patientId));
    std::string cleanName = utils::trim(name);
    std::string cleanContact = utils::trim(contact);
    int age = 0;
    checkPatientInput(id, cleanName, utils::trim(ageText), cleanContact, age);
    if (database.findPatient(id) != NULL) {
        throw model::DuplicateIdException(id);
    }

    model::PatientRecord* patient = model::createPatient(category, id, cleanName, age, cleanContact,
                                                         extra1, extra2);
    try {
        database.addPatient(patient);
    } catch (...) {
        delete patient;
        throw;
    }
    idGenerator.notePatientId(id);

    UndoAction action;
    action.type = UNDO_ADD_PATIENT;
    action.patientId = id;
    action.description = "Register patient " + id + " (" + cleanName + ")";
    undoStack.push(action);
    sortedBy = -1;
    unsavedChanges = true;
}

void ClinicManager::editPatient(const std::string& category, const std::string& patientId,
                                const std::string& name, const std::string& ageText,
                                const std::string& contact, const std::string& extra1,
                                const std::string& extra2) {
    std::string id = utils::toUpper(utils::trim(patientId));
    model::PatientRecord* patient = database.findPatient(id);
    if (patient == NULL) {
        throw model::RecordNotFoundException("No patient with ID " + id +
                                             ". Click a patient in the table first.");
    }
    if (patient->getCategory() != category) {
        throw model::InvalidInputException("The patient type cannot be changed. Delete the record and register it again as " +
                                           category + ".");
    }
    std::string cleanName = utils::trim(name);
    std::string cleanContact = utils::trim(contact);
    int age = 0;
    checkPatientInput(id, cleanName, utils::trim(ageText), cleanContact, age);

    // test the extra fields on a throw-away copy so nothing changes if they are wrong
    model::PatientRecord* test = model::createPatient(category, id, cleanName, age, cleanContact,
                                                      extra1, extra2);
    delete test;

    UndoAction action;
    action.type = UNDO_EDIT_PATIENT;
    action.patientId = id;
    action.savedPatient = utils::FileHandler::patientToLine(patient);
    action.description = "Edit patient " + id + " (" + patient->getName() + ")";

    patient->setName(cleanName);
    patient->setAge(age);
    patient->setContact(cleanContact);
    patient->setExtraFields(extra1, extra2);

    undoStack.push(action);
    sortedBy = -1;
    unsavedChanges = true;
}

void ClinicManager::deletePatient(const std::string& patientId) {
    std::string id = utils::toUpper(utils::trim(patientId));
    model::PatientRecord* patient = database.findPatient(id);
    if (patient == NULL) {
        throw model::RecordNotFoundException("No patient with ID " + id + ".");
    }

    UndoAction action;
    action.type = UNDO_DELETE_PATIENT;
    action.patientId = id;
    action.savedPatient = utils::FileHandler::patientToLine(patient);
    action.description = "Delete patient " + id + " (" + patient->getName() + ")";
    std::vector<model::Appointment*> appts = patient->getAppointments();
    for (size_t i = 0; i < appts.size(); i++) {
        action.savedAppointments.push_back(utils::FileHandler::appointmentToLine(appts[i]));
    }

    removeFromQueue(id);
    database.deletePatient(id);
    undoStack.push(action);
    unsavedChanges = true;
}

// opening a patient also puts them in the recent patients cache
model::PatientRecord* ClinicManager::openPatient(const std::string& patientId) {
    model::PatientRecord* patient = database.findPatient(utils::toUpper(utils::trim(patientId)));
    database.markAccessed(patient);
    return patient;
}

model::PatientRecord* ClinicManager::peekPatient(const std::string& patientId) const {
    return database.findPatient(utils::toUpper(utils::trim(patientId)));
}

const std::vector<model::PatientRecord*>& ClinicManager::getPatients() const {
    return database.getPatients();
}

// works for any patient type through the base class reference
std::string ClinicManager::describePatient(const model::PatientRecord& patient) const {
    std::string text = patient.getDetails();
    std::string next = patient.getNextAppointment();
    text += "\n\nNext appointment: " + (next.empty() ? std::string("none booked") : next);
    return text;
}

const model::ClinicDatabase& ClinicManager::getDatabase() const {
    return database;
}

// ---------- appointments ----------

int ClinicManager::scheduleAppointment(const std::string& patientId, const std::string& doctor,
                                       const std::string& date, const std::string& time,
                                       const std::string& service) {
    int number = appointmentController.schedule(patientId, doctor, date, time, service);
    unsavedChanges = true;
    return number;
}

void ClinicManager::cancelAppointment(int number) {
    appointmentController.cancel(number);
    unsavedChanges = true;
}

void ClinicManager::completeAppointment(int number) {
    appointmentController.complete(number);
    unsavedChanges = true;
}

void ClinicManager::removeAppointment(int number) {
    appointmentController.remove(number);
    unsavedChanges = true;
}

const std::vector<model::Appointment*>& ClinicManager::getAppointments() const {
    return database.getAppointments();
}

std::vector<model::Appointment*> ClinicManager::getAppointmentsOn(const std::string& date) const {
    return appointmentController.getAppointmentsOn(date);
}

std::vector<model::Appointment*> ClinicManager::getDueSoon(int minutesAhead) const {
    return appointmentController.getDueSoon(minutesAhead);
}

int ClinicManager::getDoctorCount() const {
    return appointmentController.getDoctorCount();
}

std::string ClinicManager::getDoctor(int index) const {
    return appointmentController.getDoctor(index);
}

// ---------- search and sort ----------

std::vector<model::PatientRecord*> ClinicManager::searchPatients(int field, int method,
                                                                 const std::string& keyword,
                                                                 std::string& methodUsed) {
    std::string key = utils::trim(keyword);
    if (key.empty()) {
        throw model::InvalidSearchException("Please type something to search for.");
    }
    if (utils::hasBadCharacters(key)) {
        throw model::InvalidSearchException("The search text contains a character that is not allowed (|).");
    }
    if (field == FIELD_CATEGORY && !searchEngine.isCategoryName(key)) {
        throw model::InvalidSearchException("Category must be General, Emergency or Follow-Up.");
    }

    const std::vector<model::PatientRecord*>& list = database.getPatients();
    std::vector<model::PatientRecord*> results;

    // auto picks the best method for the current state of the data
    int chosen = method;
    if (chosen == SEARCH_AUTO) {
        model::PatientRecord* cached = NULL;
        if (field == FIELD_ID) {
            cached = database.findInCache(utils::toUpper(key));
        }
        if (cached != NULL) {
            results.push_back(cached);
            database.markAccessed(cached);
            methodUsed = "Recent patients cache (fixed-size array)";
            return results;
        }
        if (field == FIELD_ID && sortedBy == SORT_ID) {
            chosen = SEARCH_BINARY;
        } else if (field == FIELD_NAME) {
            chosen = SEARCH_RECURSIVE;
        } else {
            chosen = SEARCH_LINEAR;
        }
    }

    if (chosen == SEARCH_BINARY) {
        if (field != FIELD_ID) {
            throw model::InvalidSearchException("Binary search only works on Patient ID. Use Linear or Recursive search for names and categories.");
        }
        if (sortedBy != SORT_ID) {
            throw model::InvalidSearchException("Binary search needs the records sorted by Patient ID first.\nUse Sort Patients with Sort By = Patient ID, then search again.");
        }
        model::PatientRecord* found = searchEngine.binarySearchById(list, utils::toUpper(key));
        if (found != NULL) {
            results.push_back(found);
        }
        methodUsed = "Binary Search (records are sorted by Patient ID)";
    } else if (chosen == SEARCH_RECURSIVE) {
        results = searchEngine.recursiveSearch(list, field, key);
        methodUsed = "Recursive Search";
    } else {
        results = searchEngine.linearSearch(list, field, key);
        methodUsed = "Linear Search";
    }

    for (int i = (int)results.size() - 1; i >= 0; i--) {
        database.markAccessed(results[i]);
    }
    return results;
}

void ClinicManager::sortPatients(int algorithm, int field) {
    std::vector<model::PatientRecord*> copy = database.getPatients();
    if (copy.size() < 2) {
        throw model::InvalidInputException("You need at least two patients before sorting.");
    }
    sortEngine.sort(copy, algorithm, field);
    database.setPatientOrder(copy);
    sortedBy = field;
    sortedWith = algorithm;
    unsavedChanges = true;
}

std::string ClinicManager::getSortState() const {
    if (sortedBy < 0) {
        return "Records are NOT sorted (registration order) - binary search is not available.";
    }
    return "Records are sorted by " + sortEngine.fieldName(sortedBy) + " (" +
           sortEngine.algorithmName(sortedWith) + ").";
}

std::string ClinicManager::describeSearch(int field, int method) const {
    if (method == SEARCH_AUTO) {
        if (field == FIELD_ID) {
            if (sortedBy == SORT_ID) {
                return "Auto will check the recent cache, then use Binary Search (data is sorted by ID).";
            }
            return "Auto will check the recent cache, then use Linear Search (data is not sorted by ID).";
        }
        if (field == FIELD_NAME) {
            return "Auto will use Recursive Search on names (part of a name is enough).";
        }
        return "Auto will use Linear Search on the category.";
    }
    if (method == SEARCH_BINARY) {
        if (field != FIELD_ID) {
            return "Binary Search only works with Patient ID.";
        }
        if (sortedBy != SORT_ID) {
            return "Binary Search needs the data sorted by Patient ID first.";
        }
        return "Binary Search is ready - the data is sorted by Patient ID.";
    }
    if (method == SEARCH_RECURSIVE) {
        return "Recursive Search checks one record then calls itself for the rest.";
    }
    return "Linear Search checks every record one by one (works on unsorted data).";
}

std::string ClinicManager::explainSort(int algorithm) const {
    return sortEngine.explain(algorithm);
}

// ---------- waiting queue ----------

void ClinicManager::addToQueue(const std::string& patientId) {
    std::string id = utils::toUpper(utils::trim(patientId));
    if (id.empty()) {
        throw model::InvalidInputException("Choose or type a patient ID to add to the queue.");
    }
    model::PatientRecord* patient = database.findPatient(id);
    if (patient == NULL) {
        throw model::RecordNotFoundException("No patient with ID " + id + ".");
    }
    std::queue<QueueEntry> copy = waitingQueue;
    while (!copy.empty()) {
        if (copy.front().patientId == id) {
            throw model::InvalidInputException(patient->getName() + " is already waiting in the queue.");
        }
        copy.pop();
    }
    QueueEntry entry;
    entry.patientId = id;
    entry.joinedAt = time(NULL);
    waitingQueue.push(entry);
    unsavedChanges = true;
}

bool ClinicManager::callNextPatient(QueueEntry& served) {
    if (waitingQueue.empty()) {
        return false;
    }
    served = waitingQueue.front();
    waitingQueue.pop();
    unsavedChanges = true;
    return true;
}

std::vector<QueueEntry> ClinicManager::getQueue() const {
    std::vector<QueueEntry> result;
    std::queue<QueueEntry> copy = waitingQueue;
    while (!copy.empty()) {
        result.push_back(copy.front());
        copy.pop();
    }
    return result;
}

int ClinicManager::getQueueSize() const {
    return (int)waitingQueue.size();
}

void ClinicManager::removeFromQueue(const std::string& patientId) {
    std::queue<QueueEntry> kept;
    while (!waitingQueue.empty()) {
        if (waitingQueue.front().patientId != patientId) {
            kept.push(waitingQueue.front());
        }
        waitingQueue.pop();
    }
    waitingQueue = kept;
}

// ---------- undo ----------

bool ClinicManager::undoLast(std::string& whatWasUndone) {
    if (undoStack.empty()) {
        return false;
    }
    UndoAction action = undoStack.top();
    undoStack.pop();

    if (action.type == UNDO_ADD_PATIENT) {
        removeFromQueue(action.patientId);
        database.deletePatient(action.patientId);
    } else if (action.type == UNDO_DELETE_PATIENT) {
        database.addPatient(utils::FileHandler::lineToPatient(action.savedPatient));
        for (size_t i = 0; i < action.savedAppointments.size(); i++) {
            database.addAppointment(utils::FileHandler::lineToAppointment(action.savedAppointments[i]));
        }
        sortedBy = -1;
    } else if (action.type == UNDO_EDIT_PATIENT) {
        model::PatientRecord* current = database.findPatient(action.patientId);
        model::PatientRecord* old = utils::FileHandler::lineToPatient(action.savedPatient);
        if (current != NULL) {
            current->setName(old->getName());
            current->setAge(old->getAge());
            current->setContact(old->getContact());
            current->setExtraFields(old->getExtra1(), old->getExtra2());
        }
        delete old;
        sortedBy = -1;
    } else if (action.type == UNDO_ADD_APPOINTMENT) {
        database.removeAppointment(action.apptNo);
    } else if (action.type == UNDO_REMOVE_APPOINTMENT) {
        database.addAppointment(utils::FileHandler::lineToAppointment(action.savedAppointments[0]));
    } else if (action.type == UNDO_STATUS_CHANGE) {
        model::Appointment* appt = database.findAppointment(action.apptNo);
        if (appt != NULL) {
            appt->setStatus(action.oldStatus);
        }
    }

    whatWasUndone = action.description;
    unsavedChanges = true;
    return true;
}

int ClinicManager::getUndoCount() const {
    return (int)undoStack.size();
}

// newest first, read from a copy so the real stack is not touched
std::vector<std::string> ClinicManager::getUndoHistory() const {
    std::vector<std::string> history;
    std::stack<UndoAction> copy = undoStack;
    while (!copy.empty()) {
        history.push_back(copy.top().description);
        copy.pop();
    }
    return history;
}

// ---------- files ----------

void ClinicManager::saveData() {
    utils::FileHandler::makeFolder(dataFolder);

    std::vector<std::string> lines;
    const std::vector<model::PatientRecord*>& patients = database.getPatients();
    for (size_t i = 0; i < patients.size(); i++) {
        lines.push_back(utils::FileHandler::patientToLine(patients[i]));
    }
    utils::FileHandler::writeLines(dataFolder + "\\patients.txt", lines);

    lines.clear();
    const std::vector<model::Appointment*>& appts = database.getAppointments();
    for (size_t i = 0; i < appts.size(); i++) {
        lines.push_back(utils::FileHandler::appointmentToLine(appts[i]));
    }
    utils::FileHandler::writeLines(dataFolder + "\\appointments.txt", lines);

    lines.clear();
    std::vector<QueueEntry> queue = getQueue();
    for (size_t i = 0; i < queue.size(); i++) {
        lines.push_back(queue[i].patientId + "|" + utils::intToStr((int)queue[i].joinedAt));
    }
    utils::FileHandler::writeLines(dataFolder + "\\queue.txt", lines);

    unsavedChanges = false;
}

// returns how many damaged lines had to be skipped
int ClinicManager::loadData() {
    newData();
    int skipped = 0;

    std::string patientFile = dataFolder + "\\patients.txt";
    if (utils::FileHandler::fileExists(patientFile)) {
        std::vector<std::string> lines = utils::FileHandler::readLines(patientFile);
        for (size_t i = 0; i < lines.size(); i++) {
            if (utils::trim(lines[i]).empty()) {
                continue;
            }
            try {
                model::PatientRecord* patient = utils::FileHandler::lineToPatient(lines[i]);
                try {
                    database.addPatient(patient);
                } catch (...) {
                    delete patient;
                    throw;
                }
                idGenerator.notePatientId(patient->getId());
            } catch (const std::exception&) {
                skipped++;
            }
        }
    }

    std::string apptFile = dataFolder + "\\appointments.txt";
    if (utils::FileHandler::fileExists(apptFile)) {
        std::vector<std::string> lines = utils::FileHandler::readLines(apptFile);
        for (size_t i = 0; i < lines.size(); i++) {
            if (utils::trim(lines[i]).empty()) {
                continue;
            }
            try {
                model::Appointment* appt = utils::FileHandler::lineToAppointment(lines[i]);
                if (database.findPatient(appt->getPatientId()) == NULL ||
                    database.findAppointment(appt->getNumber()) != NULL) {
                    delete appt;
                    skipped++;
                    continue;
                }
                database.addAppointment(appt);
                idGenerator.noteAppointmentNumber(appt->getNumber());
            } catch (const std::exception&) {
                skipped++;
            }
        }
    }

    // the queue only keeps patients who joined today
    std::string queueFile = dataFolder + "\\queue.txt";
    if (utils::FileHandler::fileExists(queueFile)) {
        std::vector<std::string> lines = utils::FileHandler::readLines(queueFile);
        for (size_t i = 0; i < lines.size(); i++) {
            std::vector<std::string> parts = utils::FileHandler::split(lines[i], '|');
            if (parts.size() != 2 || !utils::isWholeNumber(parts[1])) {
                if (!utils::trim(lines[i]).empty()) {
                    skipped++;
                }
                continue;
            }
            QueueEntry entry;
            entry.patientId = parts[0];
            entry.joinedAt = (time_t)std::atol(parts[1].c_str());
            if (database.findPatient(entry.patientId) != NULL &&
                utils::dateOf(entry.joinedAt) == utils::todayDate()) {
                waitingQueue.push(entry);
            }
        }
    }

    unsavedChanges = false;
    return skipped;
}

void ClinicManager::newData() {
    database.clearAll();
    while (!waitingQueue.empty()) {
        waitingQueue.pop();
    }
    while (!undoStack.empty()) {
        undoStack.pop();
    }
    idGenerator.reset();
    sortedBy = -1;
    sortedWith = -1;
    unsavedChanges = true;
}

void ClinicManager::exportPatients(const std::string& path) const {
    utils::FileHandler::exportPatientsCsv(path, database.getPatients());
}

int ClinicManager::importPatients(const std::string& path, int& skipped) {
    std::vector<model::PatientRecord*> imported = utils::FileHandler::importPatientsCsv(path, skipped);
    int added = 0;
    for (size_t i = 0; i < imported.size(); i++) {
        if (database.findPatient(imported[i]->getId()) != NULL) {
            delete imported[i];
            skipped++;
            continue;
        }
        database.addPatient(imported[i]);
        idGenerator.notePatientId(imported[i]->getId());
        added++;
    }
    if (added > 0) {
        sortedBy = -1;
        unsavedChanges = true;
    }
    return added;
}

bool ClinicManager::hasUnsavedChanges() const {
    return unsavedChanges;
}

}
