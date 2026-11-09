#ifndef FOLLOWUPPATIENT_H
#define FOLLOWUPPATIENT_H

#include "PatientRecord.h"

namespace model {

class FollowUpPatient : public PatientRecord {
private:
    std::string lastVisit;
    std::string referringDoctor;

public:
    FollowUpPatient(const std::string& patientId, const std::string& name, int age,
                    const std::string& contact);

    std::string getCategory() const override;
    std::string getPriorityLabel() const override;
    int getConsultationFee() const override;
    std::string getDetails() const override;

    std::string getExtraInfo() const override;
    std::string getExtra1() const override;
    std::string getExtra2() const override;
    void setExtraFields(const std::string& first, const std::string& second) override;

    std::string getLastVisit() const;
    std::string getReferringDoctor() const;
};

}

#endif
