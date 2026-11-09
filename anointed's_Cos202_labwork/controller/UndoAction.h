#ifndef UNDOACTION_H
#define UNDOACTION_H

#include <string>
#include <vector>

namespace controller {

enum UndoType {
    UNDO_ADD_PATIENT,
    UNDO_DELETE_PATIENT,
    UNDO_EDIT_PATIENT,
    UNDO_ADD_APPOINTMENT,
    UNDO_REMOVE_APPOINTMENT,
    UNDO_STATUS_CHANGE
};

// one admin operation saved on the undo stack
// records are kept as text lines so they can be rebuilt later
struct UndoAction {
    UndoType type;
    std::string description;
    std::string patientId;
    std::string savedPatient;
    std::vector<std::string> savedAppointments;
    int apptNo;
    std::string oldStatus;

    UndoAction() : type(UNDO_ADD_PATIENT), apptNo(0) {}
};

}

#endif
