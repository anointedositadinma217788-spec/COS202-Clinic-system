#ifndef GENERALPATIENT_H
#define GENERALPATIENT_H

#include "PatientRecord.h"

namespace model {

class GeneralPatient : public PatientRecord {
public:
    GeneralPatient(const std::string& patientId, const std::string& name, int age,
                   const std::string& contact);

    std::string getCategory() const override;
    std::string getPriorityLabel() const override;
    int getConsultationFee() const override;
    std::string getDetails() const override;
};

}

#endif
