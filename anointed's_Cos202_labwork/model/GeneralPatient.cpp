#include "GeneralPatient.h"

namespace model {

GeneralPatient::GeneralPatient(const std::string& patientId, const std::string& name, int age,
                               const std::string& contact)
    : PatientRecord(patientId, name, age, contact) {}

std::string GeneralPatient::getCategory() const {
    return "General";
}

std::string GeneralPatient::getPriorityLabel() const {
    return "Normal";
}

int GeneralPatient::getConsultationFee() const {
    return 3000;
}

std::string GeneralPatient::getDetails() const {
    return "GENERAL PATIENT\n\n" + PatientRecord::getDetails() +
           "\nSeen in normal order on a first come, first served basis.";
}

}
