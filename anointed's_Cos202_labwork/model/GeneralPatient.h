#ifndef GENERALPATIENT_H
#define GENERALPATIENT_H

#include "PatientRecord.h"

namespace clinic_mgmt {

class GeneralPatient : public PatientRecord {
public:
    GeneralPatient(std::string reg, std::string name, int age, std::string phone);
    ~GeneralPatient();

    void showDetails() const override;
    std::string getCategory() const override;
};

}

#endif
