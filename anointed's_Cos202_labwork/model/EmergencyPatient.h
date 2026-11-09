#ifndef EMERGENCYPATIENT_H
#define EMERGENCYPATIENT_H

#include "PatientRecord.h"
#include <string>

namespace clinic_mgmt {

class EmergencyPatient : public PatientRecord {
private:
    int priorityLevel;
    std::string complaint;

public:
    EmergencyPatient(std::string reg, std::string name, int age, std::string phone,
                     int priority, std::string comp);
    ~EmergencyPatient();

    void showDetails() const override;
    std::string getCategory() const override;

    int getPriorityLevel() const;
    std::string getComplaint() const;
};

}

#endif
