#ifndef PATIENTRECORD_H
#define PATIENTRECORD_H

#include <string>

namespace clinic_mgmt {

class PatientRecord {
protected:
    std::string regNumber;
    std::string fullName;
    int patientAge;
    std::string phoneNumber;

public:
    PatientRecord(std::string reg, std::string name, int age, std::string phone);
    virtual ~PatientRecord();

    std::string getRegNumber() const;
    std::string getFullName() const;
    int getPatientAge() const;
    std::string getPhoneNumber() const;

    void setFullName(std::string name);
    void setPatientAge(int age);
    void setPhoneNumber(std::string phone);

    virtual void showDetails() const = 0;
    virtual std::string getCategory() const = 0;
};

}

#endif
