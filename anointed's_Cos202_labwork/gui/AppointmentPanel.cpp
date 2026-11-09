#include "AppointmentPanel.h"
#include "MainWindow.h"
#include "../utils/Validator.h"
#include <cstdio>
#include <cstdlib>

namespace gui {

enum AppointmentId {
    IDC_APT_PATIENT = 100,
    IDC_APT_DOCTOR,
    IDC_APT_DATE,
    IDC_APT_TIME,
    IDC_APT_SERVICE,
    IDC_APT_SCHEDULE,
    IDC_APT_CLEAR,
    IDC_APT_NUMBER,
    IDC_APT_CANCEL,
    IDC_APT_COMPLETE,
    IDC_APT_REMOVE,
    IDC_APT_FILTER,
    IDC_APT_LIST
};

enum AppointmentAction {
    ACTION_CANCEL,
    ACTION_COMPLETE,
    ACTION_REMOVE
};

AppointmentPanel::AppointmentPanel(MainWindow* owner)
    : Panel(owner), patientInfoColour(RGB(110, 110, 110)) {}

void AppointmentPanel::buildControls() {
    BoxLayout* root = new BoxLayout(true, 8, 12);
    BoxLayout* top = new BoxLayout(false, 12);

    HWND scheduleBox = makeGroup("Schedule Appointment");
    GridLayout* form = new GridLayout(2, 24, 6);
    form->setColumnWidth(0, 135);

    patientEdit = makeEdit(IDC_APT_PATIENT, ES_UPPERCASE);
    addTip(patientEdit, "Type the ID of a registered patient, the name is checked as you type");
    form->addWidget(makeLabel("Patient ID:"), 0, 0);
    form->addWidget(patientEdit, 0, 1);
    patientInfo = makeLabel("");
    form->addWidget(patientInfo, 1, 1);

    doctorCombo = makeCombo(IDC_APT_DOCTOR, false);
    for (int i = 0; i < manager().getDoctorCount(); i++) {
        addComboItem(doctorCombo, manager().getDoctor(i));
    }
    SendMessageA(doctorCombo, CB_SETCURSEL, 0, 0);
    form->addWidget(makeLabel("Doctor:"), 2, 0);
    form->addWidget(doctorCombo, 2, 1);

    dateEdit = makeEdit(IDC_APT_DATE);
    addTip(dateEdit, "Appointment date as YYYY-MM-DD (today or later)");
    form->addWidget(makeLabel("Date (YYYY-MM-DD):"), 3, 0);
    form->addWidget(dateEdit, 3, 1);

    timeCombo = makeCombo(IDC_APT_TIME, true);
    for (int minutes = 8 * 60; minutes <= 17 * 60 + 30; minutes += 30) {
        char slot[8];
        sprintf(slot, "%02d:%02d", minutes / 60, minutes % 60);
        addComboItem(timeCombo, slot);
    }
    addTip(timeCombo, "Pick a time slot or type one in HH:MM (08:00 to 17:30)");
    form->addWidget(makeLabel("Time (HH:MM):"), 4, 0);
    form->addWidget(timeCombo, 4, 1);

    serviceCombo = makeCombo(IDC_APT_SERVICE, true);
    addComboItem(serviceCombo, "General Consultation");
    addComboItem(serviceCombo, "Emergency Care");
    addComboItem(serviceCombo, "Follow-Up Review");
    addComboItem(serviceCombo, "Laboratory Test");
    addComboItem(serviceCombo, "Vaccination");
    addComboItem(serviceCombo, "Dental Check");
    addComboItem(serviceCombo, "Eye Test");
    form->addWidget(makeLabel("Service:"), 5, 0);
    form->addWidget(serviceCombo, 5, 1);

    BoxLayout* buttons = new BoxLayout(false, 8);
    buttons->addWidget(makeButton(IDC_APT_SCHEDULE, "Schedule Appointment", "Book the appointment"), 160);
    buttons->addWidget(makeButton(IDC_APT_CLEAR, "Clear", "Reset the form"), 80);
    form->addLayout(buttons, 6, 1);
    top->addLayout(new GroupLayout(scheduleBox, form), 420, 1);

    HWND manageBox = makeGroup("Manage Selected Appointment");
    GridLayout* manage = new GridLayout(2, 26, 8);
    manage->setColumnWidth(0, 120);
    numberEdit = makeEdit(IDC_APT_NUMBER, ES_NUMBER);
    addTip(numberEdit, "Click an appointment in the table or type its number");
    manage->addWidget(makeLabel("Appointment No:"), 0, 0);
    manage->addWidget(numberEdit, 0, 1);
    manage->addWidget(makeButton(IDC_APT_CANCEL, "Cancel Appointment", "Mark the appointment as cancelled"), 1, 0, 2);
    manage->addWidget(makeButton(IDC_APT_COMPLETE, "Mark as Completed", "The patient has been seen"), 2, 0, 2);
    manage->addWidget(makeButton(IDC_APT_REMOVE, "Remove Appointment", "Delete the appointment record completely (can be undone)"), 3, 0, 2);
    filterCombo = makeCombo(IDC_APT_FILTER, false);
    addComboItem(filterCombo, "All appointments");
    addComboItem(filterCombo, "Scheduled only");
    addComboItem(filterCombo, "Completed only");
    addComboItem(filterCombo, "Cancelled only");
    addComboItem(filterCombo, "Today only");
    SendMessageA(filterCombo, CB_SETCURSEL, 0, 0);
    addTip(filterCombo, "Choose which appointments to show in the table");
    manage->addWidget(makeLabel("Show:"), 4, 0);
    manage->addWidget(filterCombo, 4, 1);
    top->addLayout(new GroupLayout(manageBox, manage), 300);

    root->addLayout(top, 232);

    root->addWidget(makeLabel("Booked appointments - click a row to select it.     "
                              "Yellow = Scheduled     Green = Completed     Grey = Cancelled"), 20);
    apptList = makeListView(IDC_APT_LIST);
    addColumn(apptList, 0, "No.", 50);
    addColumn(apptList, 1, "Patient ID", 75);
    addColumn(apptList, 2, "Patient Name", 165);
    addColumn(apptList, 3, "Category", 85);
    addColumn(apptList, 4, "Doctor", 105);
    addColumn(apptList, 5, "Date", 90);
    addColumn(apptList, 6, "Time", 55);
    addColumn(apptList, 7, "Service", 160);
    addColumn(apptList, 8, "Status", 85);
    root->addWidget(apptList, 100, 1);

    layout = root;
    clearForm();
}

void AppointmentPanel::refresh() {
    int filter = comboIndex(filterCombo);
    std::string today = utils::todayDate();
    const std::vector<model::Appointment*>& appts = manager().getAppointments();
    clearList(apptList);
    for (size_t i = 0; i < appts.size(); i++) {
        model::Appointment* a = appts[i];
        std::string status = a->getStatus();
        if ((filter == 1 && status != "Scheduled") || (filter == 2 && status != "Completed") ||
            (filter == 3 && status != "Cancelled") || (filter == 4 && a->getDate() != today)) {
            continue;
        }
        model::PatientRecord* patient = manager().peekPatient(a->getPatientId());
        std::vector<std::string> cells;
        cells.push_back(utils::intToStr(a->getNumber()));
        cells.push_back(a->getPatientId());
        cells.push_back(patient != NULL ? patient->getName() : "(unknown)");
        cells.push_back(patient != NULL ? patient->getCategory() : "-");
        cells.push_back(a->getDoctor());
        cells.push_back(a->getDate());
        cells.push_back(a->getTime());
        cells.push_back(a->getService());
        cells.push_back(status);
        addRow(apptList, cells, colourFor(status));
    }
    showPatientInfo();
}

// text field event: show who the typed patient ID belongs to
void AppointmentPanel::showPatientInfo() {
    std::string id = utils::trim(getText(patientEdit));
    if (id.empty()) {
        patientInfoColour = RGB(110, 110, 110);
        setText(patientInfo, "Type a patient ID above.");
    } else {
        model::PatientRecord* patient = manager().peekPatient(id);
        if (patient == NULL) {
            patientInfoColour = RGB(190, 30, 30);
            setText(patientInfo, "No patient with this ID.");
        } else {
            patientInfoColour = RGB(20, 130, 60);
            setText(patientInfo, patient->getName() + " - " + patient->getCategory() + " (" +
                                 patient->getPriorityLabel() + ")");
        }
    }
    InvalidateRect(patientInfo, NULL, TRUE);
}

void AppointmentPanel::clearForm() {
    setText(patientEdit, "");
    SendMessageA(doctorCombo, CB_SETCURSEL, 0, 0);
    setText(dateEdit, utils::todayDate());
    setText(timeCombo, "");
    SendMessageA(serviceCombo, CB_SETCURSEL, 0, 0);
    showPatientInfo();
}

void AppointmentPanel::schedule() {
    try {
        int doctorIndex = comboIndex(doctorCombo);
        std::string doctor = doctorIndex >= 0 ? manager().getDoctor(doctorIndex) : "";
        int number = manager().scheduleAppointment(getText(patientEdit), doctor, getText(dateEdit),
                                                   getText(timeCombo), getText(serviceCombo));
        model::Appointment* appt = manager().getDatabase().findAppointment(number);
        owner->refreshAll();
        owner->setStatus("Scheduled appointment #" + utils::intToStr(number) + ".");
        showInfo("Appointment Scheduled", "Appointment #" + utils::intToStr(number) + " booked with " +
                                          appt->getDoctor() + " on " + appt->getDate() + " at " +
                                          appt->getTime() + ".");
        clearForm();
    } catch (const std::exception& error) {
        showException(error);
    } catch (...) {
        showError("Error", "Error: Something unexpected went wrong while booking.");
    }
}

void AppointmentPanel::changeAppointment(int action) {
    try {
        std::string numberText = utils::trim(getText(numberEdit));
        if (!utils::isWholeNumber(numberText) || numberText.size() > 6) {
            showError("Invalid Appointment", "Error: Select an appointment in the table first.");
            return;
        }
        int number = std::atoi(numberText.c_str());
        std::string done;
        if (action == ACTION_CANCEL) {
            if (!askYesNo("Cancel Appointment", "Cancel appointment #" + numberText + "?")) {
                return;
            }
            manager().cancelAppointment(number);
            done = "cancelled";
        } else if (action == ACTION_COMPLETE) {
            manager().completeAppointment(number);
            done = "marked as completed";
        } else {
            if (!askYesNo("Remove Appointment", "Remove appointment #" + numberText +
                                                " from the records?\nYou can undo this with Ctrl+Z.")) {
                return;
            }
            manager().removeAppointment(number);
            done = "removed";
            setText(numberEdit, "");
        }
        owner->refreshAll();
        owner->setStatus("Appointment #" + numberText + " " + done + ".");
        showInfo("Appointments", "Appointment #" + numberText + " was " + done + ".");
    } catch (const std::exception& error) {
        showException(error);
    } catch (...) {
        showError("Error", "Error: Something unexpected went wrong.");
    }
}

void AppointmentPanel::loadSelected() {
    int row = selectedRow(apptList);
    if (row >= 0) {
        setText(numberEdit, cellText(apptList, row, 0));
        manager().openPatient(cellText(apptList, row, 1));
    }
}

void AppointmentPanel::onCommand(int id, int code) {
    if (id == IDC_APT_PATIENT && code == EN_CHANGE) {
        showPatientInfo();
        return;
    }
    if (id == IDC_APT_FILTER && code == CBN_SELCHANGE) {
        refresh();
        return;
    }
    if (code != BN_CLICKED) {
        return;
    }
    switch (id) {
    case IDC_APT_SCHEDULE:
        schedule();
        break;
    case IDC_APT_CLEAR:
        clearForm();
        break;
    case IDC_APT_CANCEL:
        changeAppointment(ACTION_CANCEL);
        break;
    case IDC_APT_COMPLETE:
        changeAppointment(ACTION_COMPLETE);
        break;
    case IDC_APT_REMOVE:
        changeAppointment(ACTION_REMOVE);
        break;
    }
}

LRESULT AppointmentPanel::onNotify(NMHDR* header) {
    if (header->hwndFrom != apptList) {
        return 0;
    }
    if (header->code == NM_CUSTOMDRAW) {
        return paintRow(header);
    }
    if (header->code == LVN_ITEMCHANGED) {
        NMLISTVIEW* change = (NMLISTVIEW*)header;
        if ((change->uNewState & LVIS_SELECTED) && !(change->uOldState & LVIS_SELECTED)) {
            loadSelected();
        }
    }
    return 0;
}

COLORREF AppointmentPanel::labelColour(HWND control) {
    if (control == patientInfo) {
        return patientInfoColour;
    }
    return Panel::labelColour(control);
}

}
