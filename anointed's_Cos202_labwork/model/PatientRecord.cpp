#include "PatientRecord.h"

namespace clinic_mgmt {

PatientRecord::PatientRecord(std::string reg, std::string name, int age, std::string phone)
    : regNumber(reg), fullName(name), patientAge(age), phoneNumber(phone) {}

PatientRecord::~PatientRecord() {}

std::string PatientRecord::getRegNumber() const { return regNumber; }
std::string PatientRecord::getFullName() const { return fullName; }
int PatientRecord::getPatientAge() const { return patientAge; }
std::string PatientRecord::getPhoneNumber() const { return phoneNumber; }

void PatientRecord::setFullName(std::string name) { fullName = name; }
void PatientRecord::setPatientAge(int age) { patientAge = age; }
void PatientRecord::setPhoneNumber(std::string phone) { phoneNumber = phone; }

}
