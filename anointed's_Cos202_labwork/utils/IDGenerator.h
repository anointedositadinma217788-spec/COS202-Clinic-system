#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <string>

namespace utils {

// hands out patient IDs like P001 and appointment numbers
class IDGenerator {
private:
    int lastPatientNumber;
    int lastAppointmentNumber;

public:
    IDGenerator();

    void reset();
    void notePatientId(const std::string& patientId);
    void noteAppointmentNumber(int number);

    std::string nextPatientId() const;
    int takeAppointmentNumber();
};

}

#endif
