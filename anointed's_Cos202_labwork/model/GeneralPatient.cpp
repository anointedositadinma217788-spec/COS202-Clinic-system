#include "GeneralPatient.h"
#include <iostream>

namespace clinic_mgmt {

GeneralPatient::GeneralPatient(std::string reg, std::string name, int age, std::string phone)
    : PatientRecord(reg, name, age, phone) {}

GeneralPatient::~GeneralPatient() {}

void GeneralPatient::showDetails() const {
    std::cout << "General Patient" << std::endl;
    std::cout << "Reg No: " << getRegNumber() << std::endl;
    std::cout << "Name: " << getFullName() << std::endl;
    std::cout << "Age: " << getPatientAge() << std::endl;
    std::cout << "Phone: " << getPhoneNumber() << std::endl;
}

std::string GeneralPatient::getCategory() const {
    return "General";
}

}
