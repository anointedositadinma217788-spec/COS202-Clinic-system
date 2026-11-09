#ifndef EMERGENCYPATIENT_H
#define EMERGENCYPATIENT_H

#include "PatientRecord.h"

namespace model {

class EmergencyPatient : public PatientRecord {
private:
    int priorityLevel;
    std::string symptoms;

public:
    EmergencyPatient(const std::string& patientId, const std::string& name, int age,
                     const std::string& contact);

    std::string getCategory() const override;
    std::string getPriorityLabel() const override;
    int getConsultationFee() const override;
    std::string getDetails() const override;

    std::string getExtraInfo() const override;
    std::string getExtra1() const override;
    std::string getExtra2() const override;
    void setExtraFields(const std::string& first, const std::string& second) override;

    int getPriorityLevel() const;
    std::string getSymptoms() const;
};

}

#endif
