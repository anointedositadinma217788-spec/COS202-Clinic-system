#include "AppointmentController.h"
#include "../model/ClinicExceptions.h"
#include "../utils/Validator.h"
#include "../utils/FileHandler.h"

namespace controller {

AppointmentController::AppointmentController(model::ClinicDatabase& database,
                                             std::stack<UndoAction>& undoStack,
                                             utils::IDGenerator& idGenerator)
    : database(database), undoStack(undoStack), idGenerator(idGenerator) {
    doctors[0] = "Dr. Adeyemi";
    doctors[1] = "Dr. Bello";
    doctors[2] = "Dr. Chukwu";
    doctors[3] = "Dr. Eze";
    doctors[4] = "Dr. Hassan";
    doctors[5] = "Dr. Okafor";
}

int AppointmentController::schedule(const std::string& patientId, const std::string& doctor,
                                    const std::string& date, const std::string& time,
                                    const std::string& service) {
    std::string id = utils::toUpper(utils::trim(patientId));
    std::string cleanDate = utils::trim(date);
    std::string cleanTime = utils::trim(time);
    std::string cleanService = utils::trim(service);

    if (id.empty()) {
        throw model::InvalidAppointmentException("Enter the ID of the patient for this appointment.");
    }
    model::PatientRecord* patient = database.findPatient(id);
    if (patient == NULL) {
        throw model::RecordNotFoundException("No patient with ID " + id + ". Register the patient first.");
    }
    if (doctor.empty()) {
        throw model::InvalidAppointmentException("Please choose a doctor.");
    }
    if (!utils::isValidDate(cleanDate)) {
        throw model::InvalidAppointmentException("Date must be a real date written as YYYY-MM-DD, for example " +
                                                 utils::todayDate() + ".");
    }
    if (cleanDate < utils::todayDate()) {
        throw model::InvalidAppointmentException("You cannot book an appointment for a date in the past.");
    }
    if (!utils::isValidTime(cleanTime)) {
        throw model::InvalidAppointmentException("Time must be in 24 hour HH:MM format, for example 09:30.");
    }
    int minutes = utils::timeToMinutes(cleanTime);
    if (minutes < 8 * 60 || minutes > 17 * 60 + 30) {
        throw model::InvalidAppointmentException("The clinic only books appointments from 08:00 to 17:30.");
    }
    if (cleanDate == utils::todayDate() && minutes < utils::timeToMinutes(utils::currentTime())) {
        throw model::InvalidAppointmentException("That time has already passed today.");
    }
    if (cleanService.empty()) {
        throw model::InvalidAppointmentException("Please enter the service the patient needs.");
    }
    if (utils::hasBadCharacters(cleanService)) {
        throw model::InvalidAppointmentException("Service cannot contain the | character.");
    }

    // no double booking for the doctor or the patient
    const std::vector<model::Appointment*>& all = database.getAppointments();
    for (size_t i = 0; i < all.size(); i++) {
        if (!all[i]->isPending() || all[i]->getDate() != cleanDate || all[i]->getTime() != cleanTime) {
            continue;
        }
        if (all[i]->getDoctor() == doctor) {
            throw model::InvalidAppointmentException(doctor + " already has an appointment on " +
                                                     cleanDate + " at " + cleanTime + ".");
        }
        if (all[i]->getPatientId() == id) {
            throw model::InvalidAppointmentException(patient->getName() +
                                                     " already has an appointment at that time.");
        }
    }

    int number = idGenerator.takeAppointmentNumber();
    database.addAppointment(new model::Appointment(number, id, doctor, cleanDate, cleanTime,
                                                   cleanService, "Scheduled"));

    UndoAction action;
    action.type = UNDO_ADD_APPOINTMENT;
    action.apptNo = number;
    action.patientId = id;
    action.description = "Schedule appointment #" + utils::intToStr(number) + " for " + id;
    undoStack.push(action);
    return number;
}

model::Appointment* AppointmentController::getScheduled(int number, const std::string& action) const {
    model::Appointment* appt = database.findAppointment(number);
    if (appt == NULL) {
        throw model::RecordNotFoundException("Appointment #" + utils::intToStr(number) +
                                             " does not exist. Select one from the table.");
    }
    if (!appt->isPending()) {
        throw model::InvalidAppointmentException("Only scheduled appointments can be " + action +
                                                 ". Appointment #" + utils::intToStr(number) +
                                                 " is already " + appt->getStatus() + ".");
    }
    return appt;
}

void AppointmentController::cancel(int number) {
    model::Appointment* appt = getScheduled(number, "cancelled");

    UndoAction action;
    action.type = UNDO_STATUS_CHANGE;
    action.apptNo = number;
    action.oldStatus = appt->getStatus();
    action.description = "Cancel appointment #" + utils::intToStr(number);

    // the patient cancels through the Appointable interface
    model::Appointable* owner = database.findPatient(appt->getPatientId());
    if (owner == NULL || !owner->cancelAppointment(number)) {
        appt->setStatus("Cancelled");
    }
    undoStack.push(action);
}

void AppointmentController::complete(int number) {
    model::Appointment* appt = getScheduled(number, "completed");

    UndoAction action;
    action.type = UNDO_STATUS_CHANGE;
    action.apptNo = number;
    action.oldStatus = appt->getStatus();
    action.description = "Mark appointment #" + utils::intToStr(number) + " completed";

    appt->setStatus("Completed");
    undoStack.push(action);
}

void AppointmentController::remove(int number) {
    model::Appointment* appt = database.findAppointment(number);
    if (appt == NULL) {
        throw model::RecordNotFoundException("Appointment #" + utils::intToStr(number) +
                                             " does not exist. Select one from the table.");
    }
    UndoAction action;
    action.type = UNDO_REMOVE_APPOINTMENT;
    action.apptNo = number;
    action.savedAppointments.push_back(utils::FileHandler::appointmentToLine(appt));
    action.description = "Remove appointment #" + utils::intToStr(number);

    database.removeAppointment(number);
    undoStack.push(action);
}

std::vector<model::Appointment*> AppointmentController::getAppointmentsOn(const std::string& date) const {
    std::vector<model::Appointment*> result;
    const std::vector<model::Appointment*>& all = database.getAppointments();
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i]->getDate() == date) {
            result.push_back(all[i]);
        }
    }
    return result;
}

// scheduled appointments today that start within the next few minutes
std::vector<model::Appointment*> AppointmentController::getDueSoon(int minutesAhead) const {
    std::vector<model::Appointment*> result;
    std::vector<model::Appointment*> today = getAppointmentsOn(utils::todayDate());
    int now = utils::timeToMinutes(utils::currentTime());
    for (size_t i = 0; i < today.size(); i++) {
        int start = utils::timeToMinutes(today[i]->getTime());
        if (today[i]->isPending() && start >= now && start - now <= minutesAhead) {
            result.push_back(today[i]);
        }
    }
    return result;
}

int AppointmentController::getDoctorCount() const {
    return DOCTOR_COUNT;
}

std::string AppointmentController::getDoctor(int index) const {
    if (index < 0 || index >= DOCTOR_COUNT) {
        return "";
    }
    return doctors[index];
}

}
