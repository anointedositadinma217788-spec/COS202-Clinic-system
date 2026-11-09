#ifndef FOLLOWUPPATIENT_H
#define FOLLOWUPPATIENT_H

#include "PatientRecord.h"
#include <string>

namespace clinic_mgmt {

class FollowUpPatient : public PatientRecord {
private:
    std::string lastVisit;
    std::string doctorName;

public:
    FollowUpPatient(std::string reg, std::string name, int age, std::string phone,
                    std::string visit, std::string doctor);
    ~FollowUpPatient();

    void showDetails() const override;
    std::string getCategory() const override;

    std::string getLastVisit() const;
    std::string getDoctorName() const;
};

}

#endif
