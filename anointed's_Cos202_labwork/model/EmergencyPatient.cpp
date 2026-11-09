#include "EmergencyPatient.h"
#include "ClinicExceptions.h"
#include "../utils/Validator.h"
#include <cstdlib>

namespace model {

EmergencyPatient::EmergencyPatient(const std::string& patientId, const std::string& name, int age,
                                   const std::string& contact)
    : PatientRecord(patientId, name, age, contact), priorityLevel(1), symptoms("") {}

std::string EmergencyPatient::getCategory() const {
    return "Emergency";
}

std::string EmergencyPatient::getPriorityLabel() const {
    return "High Priority";
}

// emergency care costs more the more serious it is
int EmergencyPatient::getConsultationFee() const {
    return 5000 + priorityLevel * 1000;
}

std::string EmergencyPatient::getDetails() const {
    return "EMERGENCY PATIENT\n\n" + PatientRecord::getDetails() +
           "Priority level: " + utils::intToStr(priorityLevel) + " of 5\n" +
           "Symptoms: " + symptoms + "\n\nMust be attended to before normal patients.";
}

std::string EmergencyPatient::getExtraInfo() const {
    return "Priority " + utils::intToStr(priorityLevel) + " - " + symptoms;
}

std::string EmergencyPatient::getExtra1() const {
    return utils::intToStr(priorityLevel);
}

std::string EmergencyPatient::getExtra2() const {
    return symptoms;
}

// first = priority level (1 to 5), second = symptoms
void EmergencyPatient::setExtraFields(const std::string& first, const std::string& second) {
    std::string level = utils::trim(first);
    if (!utils::isWholeNumber(level) || level.size() > 1 ||
        std::atoi(level.c_str()) < 1 || std::atoi(level.c_str()) > 5) {
        throw InvalidInputException("Emergency priority level must be a number from 1 to 5.");
    }
    std::string text = utils::trim(second);
    if (text.empty()) {
        throw InvalidInputException("Please describe the symptoms of the emergency patient.");
    }
    if (utils::hasBadCharacters(text)) {
        throw InvalidInputException("Symptoms cannot contain the | character.");
    }
    priorityLevel = std::atoi(level.c_str());
    symptoms = text;
}

int EmergencyPatient::getPriorityLevel() const { return priorityLevel; }
std::string EmergencyPatient::getSymptoms() const { return symptoms; }

}
