#ifndef APPOINTABLE_H
#define APPOINTABLE_H

#include <vector>

namespace model {

class Appointment;

// interface for anything that can be booked for an appointment
class Appointable {
public:
    virtual ~Appointable() {}

    virtual void scheduleAppointment(Appointment* appt) = 0;
    virtual bool cancelAppointment(int apptNo) = 0;
    virtual std::vector<Appointment*> getAppointments() const = 0;
};

}

#endif
