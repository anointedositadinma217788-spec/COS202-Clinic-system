#include "FollowUpPatient.h"
#include "ClinicExceptions.h"
#include "../utils/Validator.h"

namespace model {

FollowUpPatient::FollowUpPatient(const std::string& patientId, const std::string& name, int age,
                                 const std::string& contact)
    : PatientRecord(patientId, name, age, contact), lastVisit(""), referringDoctor("") {}

std::string FollowUpPatient::getCategory() const {
    return "Follow-Up";
}

std::string FollowUpPatient::getPriorityLabel() const {
    return "Scheduled";
}

// follow up visits get a reduced fee
int FollowUpPatient::getConsultationFee() const {
    return 1500;
}

std::string FollowUpPatient::getDetails() const {
    return "FOLLOW-UP PATIENT\n\n" + PatientRecord::getDetails() +
           "Last visit: " + lastVisit + "\n" +
           "Referred by: " + referringDoctor + "\n\nComes back on a booked date for review.";
}

std::string FollowUpPatient::getExtraInfo() const {
    return "Last visit " + lastVisit + ", ref. " + referringDoctor;
}

std::string FollowUpPatient::getExtra1() const {
    return lastVisit;
}

std::string FollowUpPatient::getExtra2() const {
    return referringDoctor;
}

// first = last visit date, second = referring doctor
void FollowUpPatient::setExtraFields(const std::string& first, const std::string& second) {
    std::string date = utils::trim(first);
    if (!utils::isValidDate(date)) {
        throw InvalidInputException("Last visit date must be a real date written as YYYY-MM-DD.");
    }
    if (date > utils::todayDate()) {
        throw InvalidInputException("Last visit date cannot be in the future.");
    }
    std::string doctor = utils::trim(second);
    if (doctor.empty()) {
        throw InvalidInputException("Please enter the referring doctor for the follow-up patient.");
    }
    if (utils::hasBadCharacters(doctor)) {
        throw InvalidInputException("Referring doctor cannot contain the | character.");
    }
    lastVisit = date;
    referringDoctor = doctor;
}

std::string FollowUpPatient::getLastVisit() const { return lastVisit; }
std::string FollowUpPatient::getReferringDoctor() const { return referringDoctor; }

}
