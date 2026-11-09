#include "ClinicDatabase.h"
#include "ClinicExceptions.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"
#include <fstream>
#include <sstream>
#include <cctype>

namespace clinic_mgmt {

// actions stored on the undo stack
const int ACT_ADD_PATIENT = 0;
const int ACT_DEL_PATIENT = 1;
const int ACT_SCHEDULE = 2;
const int ACT_CANCEL = 3;

static std::string lower(std::string s) {
    for (size_t i = 0; i < s.size(); i++) {
        s[i] = (char)std::tolower((unsigned char)s[i]);
    }
    return s;
}

ClinicDatabase::ClinicDatabase() {
    nextApptNum = 1;
    doctorCount = MAX_DOCTORS;
    doctorList[0] = "Dr. Adeyemi";
    doctorList[1] = "Dr. Bello";
    doctorList[2] = "Dr. Chukwu";
    doctorList[3] = "Dr. Danladi";
    doctorList[4] = "Dr. Eze";
    doctorList[5] = "Dr. Falana";
    doctorList[6] = "Dr. Grace";
    doctorList[7] = "Dr. Hassan";
}

ClinicDatabase::~ClinicDatabase() {
    for (size_t i = 0; i < patientList.size(); i++) {
        delete patientList[i];
    }
    for (size_t i = 0; i < apptList.size(); i++) {
        delete apptList[i];
    }
}

void ClinicDatabase::addPatient(PatientRecord* patient) {
    if (patient->getRegNumber().empty() || patient->getFullName().empty()) {
        delete patient;
        throw InvalidInputException("Registration number and name cannot be empty.");
    }
    if (patient->getPatientAge() < 1 || patient->getPatientAge() > 120) {
        int badAge = patient->getPatientAge();
        delete patient;
        throw InvalidInputException("Age must be between 1 and 120.");
    }
    for (size_t i = 0; i < patientList.size(); i++) {
        if (patientList[i]->getRegNumber() == patient->getRegNumber()) {
            std::string dup = patient->getRegNumber();
            delete patient;
            throw DuplicateIdException(dup);
        }
    }
    patientList.push_back(patient);

    UndoEntry entry;
    entry.action = ACT_ADD_PATIENT;
    entry.patientType = patient->getCategory();
    entry.reg = patient->getRegNumber();
    entry.name = patient->getFullName();
    entry.age = patient->getPatientAge();
    entry.contact = patient->getPhoneNumber();
    entry.field1 = "";
    entry.field2 = "";
    entry.apptNum = 0;
    undoStack.push(entry);
    saveAll();
}

bool ClinicDatabase::editPatient(std::string reg, std::string newName, int newAge,
                                 std::string newContact) {
    PatientRecord* patient = findPatientByReg(reg);
    if (!patient) {
        return false;
    }
    if (newName.empty()) {
        throw InvalidInputException("Name cannot be empty.");
    }
    if (newAge < 1 || newAge > 120) {
        throw InvalidInputException("Age must be between 1 and 120.");
    }
    patient->setFullName(newName);
    patient->setPatientAge(newAge);
    patient->setPhoneNumber(newContact);
    saveAll();
    return true;
}

bool ClinicDatabase::removePatient(std::string reg) {
    for (size_t i = 0; i < patientList.size(); i++) {
        if (patientList[i]->getRegNumber() == reg) {
            PatientRecord* patient = patientList[i];
            UndoEntry entry;
            entry.action = ACT_DEL_PATIENT;
            entry.patientType = patient->getCategory();
            entry.reg = patient->getRegNumber();
            entry.name = patient->getFullName();
            entry.age = patient->getPatientAge();
            entry.contact = patient->getPhoneNumber();
            entry.apptNum = 0;
            if (entry.patientType == "Emergency") {
                EmergencyPatient* ep = dynamic_cast<EmergencyPatient*>(patient);
                if (ep) {
                    entry.field1 = ep->getComplaint();
                    std::stringstream sstream;
                    sstream << ep->getPriorityLevel();
                    entry.field2 = sstream.str();
                }
            } else if (entry.patientType == "FollowUp") {
                FollowUpPatient* fp = dynamic_cast<FollowUpPatient*>(patient);
                if (fp) {
                    entry.field1 = fp->getLastVisit();
                    entry.field2 = fp->getDoctorName();
                }
            }
            delete patient;
            patientList.erase(patientList.begin() + i);
            undoStack.push(entry);
            saveAll();
            return true;
        }
    }
    return false;
}

PatientRecord* ClinicDatabase::findPatientByReg(std::string reg) const {
    for (size_t i = 0; i < patientList.size(); i++) {
        if (patientList[i]->getRegNumber() == reg) {
            return patientList[i];
        }
    }
    return nullptr;
}

std::vector<PatientRecord*> ClinicDatabase::getAllPatients() const {
    return patientList;
}

// manual linear search over the patients, case-insensitive partial match
std::vector<PatientRecord*> ClinicDatabase::linearSearchByName(std::string name) const {
    std::vector<PatientRecord*> found;
    std::string target = lower(name);
    for (size_t i = 0; i < patientList.size(); i++) {
        std::string current = lower(patientList[i]->getFullName());
        if (current.find(target) != std::string::npos) {
            found.push_back(patientList[i]);
        }
    }
    return found;
}

// sorts a copy of the list by reg number, the binary search itself is recursive
PatientRecord* ClinicDatabase::binarySearchByReg(std::string reg) {
    std::vector<PatientRecord*> copyList = patientList;
    selectionSortByReg(copyList);
    return binarySearchRec(copyList, 0, (long)copyList.size() - 1, reg);
}

PatientRecord* ClinicDatabase::binarySearchRec(std::vector<PatientRecord*>& list,
                                               long low, long high,
                                               const std::string& reg) const {
    if (low > high) {
        return nullptr;
    }
    long mid = low + (high - low) / 2;
    if (list[mid]->getRegNumber() == reg) {
        return list[mid];
    }
    if (list[mid]->getRegNumber() < reg) {
        return binarySearchRec(list, mid + 1, high, reg);
    }
    return binarySearchRec(list, low, mid - 1, reg);
}

// manual selection sort, keeps swapping the smallest age into place
void ClinicDatabase::selectionSortByAgePublic(std::vector<PatientRecord*>& list) {
    for (size_t i = 0; i + 1 < list.size(); i++) {
        size_t smallest = i;
        for (size_t j = i + 1; j < list.size(); j++) {
            if (list[j]->getPatientAge() < list[smallest]->getPatientAge()) {
                smallest = j;
            }
        }
        if (smallest != i) {
            PatientRecord* temp = list[i];
            list[i] = list[smallest];
            list[smallest] = temp;
        }
    }
}

void ClinicDatabase::selectionSortByReg(std::vector<PatientRecord*>& list) const {
    for (size_t i = 0; i + 1 < list.size(); i++) {
        size_t smallest = i;
        for (size_t j = i + 1; j < list.size(); j++) {
            if (list[j]->getRegNumber() < list[smallest]->getRegNumber()) {
                smallest = j;
            }
        }
        if (smallest != i) {
            PatientRecord* temp = list[i];
            list[i] = list[smallest];
            list[smallest] = temp;
        }
    }
}

// manual insertion sort on names
void ClinicDatabase::insertionSortByName(std::vector<PatientRecord*>& list) {
    for (size_t i = 1; i < list.size(); i++) {
        PatientRecord* key = list[i];
        long j = (long)i - 1;
        while (j >= 0 && lower(list[j]->getFullName()) > lower(key->getFullName())) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = key;
    }
}

void ClinicDatabase::addAppointment(std::string reg, std::string doctor, std::string time,
                                    std::string service) {
    PatientRecord* patient = findPatientByReg(reg);
    if (!patient) {
        throw RecordNotFoundException("No patient with reg number " + reg);
    }
    if (time.empty() || service.empty()) {
        throw InvalidInputException("Appointment time and service are required.");
    }
    Appointment* appt = new Appointment(nextApptNum, reg, doctor, time, service, "Scheduled");
    apptList.push_back(appt);

    UndoEntry entry;
    entry.action = ACT_SCHEDULE;
    entry.apptNum = nextApptNum;
    entry.reg = reg;
    nextApptNum++;
    undoStack.push(entry);
    saveAll();
}

void ClinicDatabase::removeAppointmentDirect(int apptNum) {
    for (size_t i = 0; i < apptList.size(); i++) {
        if (apptList[i]->getApptNumber() == apptNum) {
            delete apptList[i];
            apptList.erase(apptList.begin() + i);
            return;
        }
    }
}

bool ClinicDatabase::cancelAppointment(int apptNum) {
    for (size_t i = 0; i < apptList.size(); i++) {
        if (apptList[i]->getApptNumber() == apptNum) {
            apptList[i]->setApptStatus("Cancelled");
            UndoEntry entry;
            entry.action = ACT_CANCEL;
            entry.apptNum = apptNum;
            undoStack.push(entry);
            saveAll();
            return true;
        }
    }
    return false;
}

std::vector<Appointment*> ClinicDatabase::getAllAppointments() const {
    return apptList;
}

PatientRecord* ClinicDatabase::recreatePatient(const UndoEntry& entry) const {
    if (entry.patientType == "Emergency") {
        int priority = entry.field2.empty() ? 1 : std::atoi(entry.field2.c_str());
        return new EmergencyPatient(entry.reg, entry.name, entry.age,
                                    entry.contact, priority, entry.field1);
    }
    if (entry.patientType == "FollowUp") {
        return new FollowUpPatient(entry.reg, entry.name, entry.age,
                                   entry.contact, entry.field1, entry.field2);
    }
    return new GeneralPatient(entry.reg, entry.name, entry.age, entry.contact);
}

bool ClinicDatabase::undoLastAction() {
    if (undoStack.empty()) {
        return false;
    }
    UndoEntry entry = undoStack.top();
    undoStack.pop();

    if (entry.action == ACT_ADD_PATIENT) {
        for (size_t i = 0; i < patientList.size(); i++) {
            if (patientList[i]->getRegNumber() == entry.reg) {
                delete patientList[i];
                patientList.erase(patientList.begin() + i);
                break;
            }
        }
    } else if (entry.action == ACT_DEL_PATIENT) {
        patientList.push_back(recreatePatient(entry));
    } else if (entry.action == ACT_SCHEDULE) {
        removeAppointmentDirect(entry.apptNum);
    } else if (entry.action == ACT_CANCEL) {
        for (size_t i = 0; i < apptList.size(); i++) {
            if (apptList[i]->getApptNumber() == entry.apptNum) {
                apptList[i]->setApptStatus("Scheduled");
                break;
            }
        }
    }
    saveAll();
    return true;
}

void ClinicDatabase::addToQueue(std::string reg) {
    PatientRecord* patient = findPatientByReg(reg);
    if (!patient) {
        throw RecordNotFoundException("No patient with reg number " + reg);
    }
    std::queue<std::string> scan = waitingQueue;
    while (!scan.empty()) {
        if (scan.front() == reg) {
            throw DuplicateIdException(reg + " is already in the queue");
        }
        scan.pop();
    }
    waitingQueue.push(reg);
}

bool ClinicDatabase::callNextPatient(std::string& result) {
    if (waitingQueue.empty()) {
        return false;
    }
    std::string reg = waitingQueue.front();
    waitingQueue.pop();
    PatientRecord* patient = findPatientByReg(reg);
    if (patient) {
        result = patient->getRegNumber() + " - " + patient->getFullName();
    } else {
        result = reg;
    }
    return true;
}

std::vector<std::string> ClinicDatabase::queueSnapshot() const {
    std::vector<std::string> items;
    std::queue<std::string> copy = waitingQueue;
    while (!copy.empty()) {
        items.push_back(copy.front());
        copy.pop();
    }
    return items;
}

int ClinicDatabase::countCategoryRecursive(size_t index, const std::string& category) const {
    if (index >= patientList.size()) {
        return 0;
    }
    int thisOne = (patientList[index]->getCategory() == category) ? 1 : 0;
    return thisOne + countCategoryRecursive(index + 1, category);
}

// recursion is also demonstrated through this public helper
int ClinicDatabase::recursiveCountByCategory(const std::string& category) const {
    return countCategoryRecursive(0, category);
}

std::string ClinicDatabase::getDoctor(int index) const {
    if (index < 0 || index >= doctorCount) {
        return "";
    }
    return doctorList[index];
}

void ClinicDatabase::saveAll() {
    try {
        std::ofstream pfile("patients.txt");
        if (!pfile.is_open()) {
            throw FileErrorException("Could not open patients.txt for saving.");
        }
        for (size_t i = 0; i < patientList.size(); i++) {
            std::string category = patientList[i]->getCategory();
            pfile << category << "|"
                  << patientList[i]->getRegNumber() << "|"
                  << patientList[i]->getFullName() << "|"
                  << patientList[i]->getPatientAge() << "|"
                  << patientList[i]->getPhoneNumber() << "|";
            if (category == "Emergency") {
                EmergencyPatient* ep = dynamic_cast<EmergencyPatient*>(patientList[i]);
                if (ep) {
                    pfile << ep->getPriorityLevel() << "|" << ep->getComplaint();
                }
            } else if (category == "FollowUp") {
                FollowUpPatient* fp = dynamic_cast<FollowUpPatient*>(patientList[i]);
                if (fp) {
                    pfile << fp->getLastVisit() << "|" << fp->getDoctorName();
                }
            }
            pfile << "\n";
        }
        pfile.close();

        std::ofstream afile("appointments.txt");
        if (!afile.is_open()) {
            throw FileErrorException("Could not open appointments.txt for saving.");
        }
        for (size_t i = 0; i < apptList.size(); i++) {
            afile << apptList[i]->getApptNumber() << "|"
                  << apptList[i]->getPatientReg() << "|"
                  << apptList[i]->getDoctor() << "|"
                  << apptList[i]->getAppointmentTime() << "|"
                  << apptList[i]->getServiceType() << "|"
                  << apptList[i]->getApptStatus() << "\n";
        }
        afile.close();
    } catch (FileErrorException&) {
        throw;
    }
}

void ClinicDatabase::loadAll() {
    std::ifstream pfile("patients.txt");
    if (pfile.is_open()) {
        std::string line;
        while (std::getline(pfile, line)) {
            if (line.empty()) {
                continue;
            }
            try {
                std::stringstream ss(line);
                std::string category, reg, name, contact, extra1, extra2;
                std::string ageStr;
                int age;
                std::getline(ss, category, '|');
                std::getline(ss, reg, '|');
                std::getline(ss, name, '|');
                std::getline(ss, ageStr, '|');
                age = std::atoi(ageStr.c_str());
                std::getline(ss, contact, '|');
                if (category == "General") {
                    patientList.push_back(new GeneralPatient(reg, name, age, contact));
                } else if (category == "Emergency") {
                    std::getline(ss, extra1, '|');
                    std::getline(ss, extra2);
                    int priority = std::atoi(extra1.c_str());
                    patientList.push_back(new EmergencyPatient(reg, name, age, contact, priority, extra2));
                } else if (category == "FollowUp") {
                    std::getline(ss, extra1, '|');
                    std::getline(ss, extra2);
                    patientList.push_back(new FollowUpPatient(reg, name, age, contact, extra1, extra2));
                }
            } catch (...) {
                // skip broken lines so the clinic still opens
            }
        }
        pfile.close();
    }

    std::ifstream afile("appointments.txt");
    if (afile.is_open()) {
        std::string line;
        while (std::getline(afile, line)) {
            if (line.empty()) {
                continue;
            }
            try {
                std::stringstream ss(line);
                std::string numStr, reg, doctor, time, service, status;
                std::getline(ss, numStr, '|');
                std::getline(ss, reg, '|');
                std::getline(ss, doctor, '|');
                std::getline(ss, time, '|');
                std::getline(ss, service, '|');
                std::getline(ss, status);
                int num = std::atoi(numStr.c_str());
                if (num > 0) {
                    apptList.push_back(new Appointment(num, reg, doctor, time, service, status));
                    if (num >= nextApptNum) {
                        nextApptNum = num + 1;
                    }
                }
            } catch (...) {
                // skip broken lines so the clinic still opens
            }
        }
        afile.close();
    }
}

}
