#include "PatientFactory.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"
#include "ClinicExceptions.h"

namespace model {

PatientRecord* createPatient(const std::string& category, const std::string& patientId,
                             const std::string& name, int age, const std::string& contact,
                             const std::string& extra1, const std::string& extra2) {
    PatientRecord* patient = NULL;
    if (category == "General") {
        patient = new GeneralPatient(patientId, name, age, contact);
    } else if (category == "Emergency") {
        patient = new EmergencyPatient(patientId, name, age, contact);
    } else if (category == "Follow-Up") {
        patient = new FollowUpPatient(patientId, name, age, contact);
    } else {
        throw InvalidInputException("Unknown patient type: " + category);
    }

    try {
        patient->setExtraFields(extra1, extra2);
    } catch (...) {
        delete patient;
        throw;
    }
    return patient;
}

}
