#include "IDGenerator.h"
#include "Validator.h"
#include <cstdio>
#include <cstdlib>

namespace utils {

IDGenerator::IDGenerator() {
    reset();
}

void IDGenerator::reset() {
    lastPatientNumber = 0;
    lastAppointmentNumber = 0;
}

// remembers the biggest number used so new IDs never clash
void IDGenerator::notePatientId(const std::string& patientId) {
    if (patientId.size() < 2 || patientId[0] != 'P') {
        return;
    }
    std::string digits = patientId.substr(1);
    if (isWholeNumber(digits) && digits.size() < 7) {
        int number = std::atoi(digits.c_str());
        if (number > lastPatientNumber) {
            lastPatientNumber = number;
        }
    }
}

void IDGenerator::noteAppointmentNumber(int number) {
    if (number > lastAppointmentNumber) {
        lastAppointmentNumber = number;
    }
}

std::string IDGenerator::nextPatientId() const {
    char buffer[16];
    sprintf(buffer, "P%03d", lastPatientNumber + 1);
    return buffer;
}

int IDGenerator::takeAppointmentNumber() {
    lastAppointmentNumber++;
    return lastAppointmentNumber;
}

}
