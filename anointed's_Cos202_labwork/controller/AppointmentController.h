#ifndef APPOINTMENTCONTROLLER_H
#define APPOINTMENTCONTROLLER_H

#include "UndoAction.h"
#include "../model/ClinicDatabase.h"
#include "../utils/IDGenerator.h"
#include <stack>
#include <string>
#include <vector>

namespace controller {

class AppointmentController {
private:
    model::ClinicDatabase& database;
    std::stack<UndoAction>& undoStack;
    utils::IDGenerator& idGenerator;

    static const int DOCTOR_COUNT = 6;
    std::string doctors[DOCTOR_COUNT];

    model::Appointment* getScheduled(int number, const std::string& action) const;

public:
    AppointmentController(model::ClinicDatabase& database, std::stack<UndoAction>& undoStack,
                          utils::IDGenerator& idGenerator);

    int schedule(const std::string& patientId, const std::string& doctor, const std::string& date,
                 const std::string& time, const std::string& service);
    void cancel(int number);
    void complete(int number);
    void remove(int number);

    std::vector<model::Appointment*> getAppointmentsOn(const std::string& date) const;
    std::vector<model::Appointment*> getDueSoon(int minutesAhead) const;

    int getDoctorCount() const;
    std::string getDoctor(int index) const;
};

}

#endif
