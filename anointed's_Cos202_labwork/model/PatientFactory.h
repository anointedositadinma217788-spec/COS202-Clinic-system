#ifndef PATIENTFACTORY_H
#define PATIENTFACTORY_H

#include "PatientRecord.h"
#include <string>

namespace model {

// builds the right subclass from the category name
PatientRecord* createPatient(const std::string& category, const std::string& patientId,
                             const std::string& name, int age, const std::string& contact,
                             const std::string& extra1, const std::string& extra2);

}

#endif
