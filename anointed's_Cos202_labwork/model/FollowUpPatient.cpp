#include "FollowUpPatient.h"
#include <iostream>

namespace clinic_mgmt {

FollowUpPatient::FollowUpPatient(std::string reg, std::string name, int age, std::string phone,
                                 std::string visit, std::string doctor)
    : PatientRecord(reg, name, age, phone), lastVisit(visit), doctorName(doctor) {}

FollowUpPatient::~FollowUpPatient() {}

void FollowUpPatient::showDetails() const {
    std::cout << "Follow-Up Patient" << std::endl;
    std::cout << "Reg No: " << getRegNumber() << std::endl;
    std::cout << "Name: " << getFullName() << std::endl;
    std::cout << "Age: " << getPatientAge() << std::endl;
    std::cout << "Phone: " << getPhoneNumber() << std::endl;
    std::cout << "Last Visit: " << lastVisit << std::endl;
    std::cout << "Doctor: " << doctorName << std::endl;
}

std::string FollowUpPatient::getCategory() const {
    return "FollowUp";
}

std::string FollowUpPatient::getLastVisit() const { return lastVisit; }
std::string FollowUpPatient::getDoctorName() const { return doctorName; }

}
