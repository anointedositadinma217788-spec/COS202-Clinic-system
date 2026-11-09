#include "EmergencyPatient.h"
#include <iostream>

namespace clinic_mgmt {

EmergencyPatient::EmergencyPatient(std::string reg, std::string name, int age, std::string phone,
                                   int priority, std::string comp)
    : PatientRecord(reg, name, age, phone), priorityLevel(priority), complaint(comp) {}

EmergencyPatient::~EmergencyPatient() {}

void EmergencyPatient::showDetails() const {
    std::cout << "Emergency Patient" << std::endl;
    std::cout << "Reg No: " << getRegNumber() << std::endl;
    std::cout << "Name: " << getFullName() << std::endl;
    std::cout << "Age: " << getPatientAge() << std::endl;
    std::cout << "Phone: " << getPhoneNumber() << std::endl;
    std::cout << "Priority: " << priorityLevel << std::endl;
    std::cout << "Complaint: " << complaint << std::endl;
}

std::string EmergencyPatient::getCategory() const {
    return "Emergency";
}

int EmergencyPatient::getPriorityLevel() const { return priorityLevel; }
std::string EmergencyPatient::getComplaint() const { return complaint; }

}
