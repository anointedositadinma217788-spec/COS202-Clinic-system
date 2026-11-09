#ifndef APPOINTABLE_H
#define APPOINTABLE_H

#include <string>
#include <vector>

namespace clinic_mgmt {

class Appointment;

class Appointable {
public:
    virtual ~Appointable() {}

    virtual void scheduleAppointment(Appointment* appt) = 0;
    virtual void cancelAppointment(int apptId) = 0;
    virtual std::vector<Appointment*> getAppointments() const = 0;
};

}

#endif
