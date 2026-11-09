#include "PatientPanel.h"
#include "MainWindow.h"
#include "../utils/Validator.h"
#include <cctype>
#include <cstdlib>

namespace gui {

enum PatientId {
    IDC_PAT_ID = 100,
    IDC_PAT_NEWID,
    IDC_PAT_NAME,
    IDC_PAT_AGE,
    IDC_PAT_CONTACT,
    IDC_PAT_TYPE,
    IDC_PAT_PRIORITY,
    IDC_PAT_SYMPTOMS,
    IDC_PAT_LASTVISIT,
    IDC_PAT_REFER,
    IDC_PAT_ADD,
    IDC_PAT_UPDATE,
    IDC_PAT_DELETE,
    IDC_PAT_DETAILS,
    IDC_PAT_QUEUE,
    IDC_PAT_CLEAR,
    IDC_PAT_LIST
};

PatientPanel::PatientPanel(MainWindow* owner)
    : Panel(owner), idStatusColour(RGB(110, 110, 110)), fillingForm(false) {}

void PatientPanel::buildControls() {
    BoxLayout* root = new BoxLayout(true, 8, 12);
    BoxLayout* top = new BoxLayout(false, 12);

    // form on the left, a grid with labels in the first column
    HWND formBox = makeGroup("Patient Details");
    GridLayout* form = new GridLayout(2, 24, 6);
    form->setColumnWidth(0, 155);

    idEdit = makeEdit(IDC_PAT_ID, ES_UPPERCASE);
    addTip(idEdit, "Unique patient ID, letters and numbers only (for example P011)");
    BoxLayout* idRow = new BoxLayout(false, 6);
    idRow->addWidget(idEdit, 100, 1);
    idRow->addWidget(makeButton(IDC_PAT_NEWID, "New ID", "Fill in the next free patient ID"), 80);
    form->addWidget(makeLabel("Patient ID:"), 0, 0);
    form->addLayout(idRow, 0, 1);

    nameEdit = makeEdit(IDC_PAT_NAME);
    addTip(nameEdit, "Full name of the patient");
    form->addWidget(makeLabel("Full Name:"), 1, 0);
    form->addWidget(nameEdit, 1, 1);

    ageEdit = makeEdit(IDC_PAT_AGE, ES_NUMBER);
    SendMessageA(ageEdit, EM_LIMITTEXT, 3, 0);
    addTip(ageEdit, "Age in years, from 0 to 120");
    form->addWidget(makeLabel("Age:"), 2, 0);
    form->addWidget(ageEdit, 2, 1);

    contactEdit = makeEdit(IDC_PAT_CONTACT);
    addTip(contactEdit, "Phone number, 7 to 15 digits");
    form->addWidget(makeLabel("Contact (Phone):"), 3, 0);
    form->addWidget(contactEdit, 3, 1);

    typeCombo = makeCombo(IDC_PAT_TYPE, false);
    addComboItem(typeCombo, "General");
    addComboItem(typeCombo, "Emergency");
    addComboItem(typeCombo, "Follow-Up");
    SendMessageA(typeCombo, CB_SETCURSEL, 0, 0);
    addTip(typeCombo, "Changing the type shows the extra fields for that type");
    form->addWidget(makeLabel("Patient Type:"), 4, 0);
    form->addWidget(typeCombo, 4, 1);

    priorityLabel = makeLabel("Priority Level:");
    priorityCombo = makeCombo(IDC_PAT_PRIORITY, false);
    addComboItem(priorityCombo, "1 - Low");
    addComboItem(priorityCombo, "2 - Moderate");
    addComboItem(priorityCombo, "3 - Serious");
    addComboItem(priorityCombo, "4 - Severe");
    addComboItem(priorityCombo, "5 - Critical");
    SendMessageA(priorityCombo, CB_SETCURSEL, 2, 0);
    form->addWidget(priorityLabel, 5, 0);
    form->addWidget(priorityCombo, 5, 1);

    symptomsLabel = makeLabel("Symptoms:");
    symptomsEdit = makeEdit(IDC_PAT_SYMPTOMS);
    form->addWidget(symptomsLabel, 6, 0);
    form->addWidget(symptomsEdit, 6, 1);

    lastVisitLabel = makeLabel("Last Visit (YYYY-MM-DD):");
    lastVisitEdit = makeEdit(IDC_PAT_LASTVISIT);
    addTip(lastVisitEdit, "Date of the previous visit, for example 2026-10-20");
    form->addWidget(lastVisitLabel, 7, 0);
    form->addWidget(lastVisitEdit, 7, 1);

    referLabel = makeLabel("Referring Doctor:");
    referEdit = makeEdit(IDC_PAT_REFER);
    form->addWidget(referLabel, 8, 0);
    form->addWidget(referEdit, 8, 1);

    idStatus = makeLabel("");
    form->addWidget(idStatus, 9, 0, 2);

    top->addLayout(new GroupLayout(formBox, form), 420, 1);

    // action buttons in a column
    HWND actionBox = makeGroup("Actions");
    BoxLayout* actions = new BoxLayout(true, 8);
    actions->addWidget(makeButton(IDC_PAT_ADD, "Add Patient", "Register a new patient with the details in the form"), 30);
    actions->addWidget(makeButton(IDC_PAT_UPDATE, "Update Patient", "Save changes to the patient with this ID"), 30);
    actions->addWidget(makeButton(IDC_PAT_DELETE, "Delete Patient", "Delete this patient and their appointments (can be undone)"), 30);
    actions->addWidget(makeButton(IDC_PAT_DETAILS, "View Details", "Show all details of the selected patient"), 30);
    actions->addWidget(makeButton(IDC_PAT_QUEUE, "Add to Waiting Queue", "Put this patient at the back of the waiting queue"), 30);
    actions->addWidget(makeButton(IDC_PAT_CLEAR, "Clear Form", "Empty the form to register someone new"), 30);
    top->addLayout(new GroupLayout(actionBox, actions), 200);

    root->addLayout(top, 300);

    root->addWidget(makeLabel("Registered patients - click a row to load it into the form.     "
                              "Red = Emergency (High Priority)     Blue = Follow-Up (Scheduled)     "
                              "White = General (Normal)"), 20);
    patientList = makeListView(IDC_PAT_LIST);
    addColumn(patientList, 0, "Patient ID", 75);
    addColumn(patientList, 1, "Name", 165);
    addColumn(patientList, 2, "Age", 45);
    addColumn(patientList, 3, "Contact", 110);
    addColumn(patientList, 4, "Category", 85);
    addColumn(patientList, 5, "Priority", 90);
    addColumn(patientList, 6, "Appts", 50);
    addColumn(patientList, 7, "Next Appointment", 125);
    addColumn(patientList, 8, "Extra Information", 260);
    root->addWidget(patientList, 100, 1);

    layout = root;
    clearForm();
}

void PatientPanel::refresh() {
    const std::vector<model::PatientRecord*>& patients = manager().getPatients();
    clearList(patientList);
    for (size_t i = 0; i < patients.size(); i++) {
        model::PatientRecord* p = patients[i];
        std::string next = p->getNextAppointment();
        std::vector<std::string> cells;
        cells.push_back(p->getId());
        cells.push_back(p->getName());
        cells.push_back(utils::intToStr(p->getAge()));
        cells.push_back(p->getContact());
        cells.push_back(p->getCategory());
        cells.push_back(p->getPriorityLabel());
        cells.push_back(utils::intToStr((int)p->getAppointments().size()));
        cells.push_back(next.empty() ? "-" : next);
        cells.push_back(p->getExtraInfo());
        addRow(patientList, cells, colourFor(p->getCategory()));
    }
    // empty form gets the next free ID (also after loading or New)
    if (utils::trim(getText(nameEdit)).empty()) {
        fillingForm = true;
        setText(idEdit, manager().suggestPatientId());
        fillingForm = false;
    }
    checkIdField();
}

std::string PatientPanel::selectedType() const {
    int index = comboIndex(typeCombo);
    if (index == 1) {
        return "Emergency";
    }
    if (index == 2) {
        return "Follow-Up";
    }
    return "General";
}

void PatientPanel::readExtras(std::string& extra1, std::string& extra2) const {
    std::string type = selectedType();
    extra1 = "";
    extra2 = "";
    if (type == "Emergency") {
        extra1 = utils::intToStr(comboIndex(priorityCombo) + 1);
        extra2 = getText(symptomsEdit);
    } else if (type == "Follow-Up") {
        extra1 = getText(lastVisitEdit);
        extra2 = getText(referEdit);
    }
}

// dynamic fields: only show what the chosen patient type needs
void PatientPanel::updateTypeFields() {
    std::string type = selectedType();
    int emergency = type == "Emergency" ? SW_SHOW : SW_HIDE;
    int followUp = type == "Follow-Up" ? SW_SHOW : SW_HIDE;
    ShowWindow(priorityLabel, emergency);
    ShowWindow(priorityCombo, emergency);
    ShowWindow(symptomsLabel, emergency);
    ShowWindow(symptomsEdit, emergency);
    ShowWindow(lastVisitLabel, followUp);
    ShowWindow(lastVisitEdit, followUp);
    ShowWindow(referLabel, followUp);
    ShowWindow(referEdit, followUp);
    relayout();
}

// text field event: tells the user straight away if the ID is free
void PatientPanel::checkIdField() {
    std::string id = utils::toUpper(utils::trim(getText(idEdit)));
    bool goodCharacters = true;
    for (size_t i = 0; i < id.size(); i++) {
        if (!std::isalnum((unsigned char)id[i])) {
            goodCharacters = false;
        }
    }
    if (id.empty()) {
        idStatusColour = RGB(110, 110, 110);
        setText(idStatus, "Type a patient ID or press New ID.");
    } else if (!goodCharacters || id.size() > 10) {
        idStatusColour = RGB(190, 30, 30);
        setText(idStatus, "ID must be up to 10 letters or numbers.");
    } else if (manager().peekPatient(id) != NULL) {
        idStatusColour = RGB(200, 110, 0);
        setText(idStatus, "ID " + id + " is registered - use Update or Delete.");
    } else {
        idStatusColour = RGB(20, 130, 60);
        setText(idStatus, "ID " + id + " is free - ready to add.");
    }
    InvalidateRect(idStatus, NULL, TRUE);
}

void PatientPanel::clearForm() {
    fillingForm = true;
    setText(idEdit, manager().suggestPatientId());
    setText(nameEdit, "");
    setText(ageEdit, "");
    setText(contactEdit, "");
    SendMessageA(typeCombo, CB_SETCURSEL, 0, 0);
    SendMessageA(priorityCombo, CB_SETCURSEL, 2, 0);
    setText(symptomsEdit, "");
    setText(lastVisitEdit, "");
    setText(referEdit, "");
    fillingForm = false;
    LVITEMA noSelection;
    ZeroMemory(&noSelection, sizeof(noSelection));
    noSelection.stateMask = LVIS_SELECTED;
    noSelection.state = 0;
    SendMessageA(patientList, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&noSelection);
    updateTypeFields();
    checkIdField();
}

void PatientPanel::loadSelected() {
    int row = selectedRow(patientList);
    if (row < 0) {
        return;
    }
    model::PatientRecord* patient = manager().openPatient(cellText(patientList, row, 0));
    if (patient == NULL) {
        return;
    }
    fillingForm = true;
    setText(idEdit, patient->getId());
    setText(nameEdit, patient->getName());
    setText(ageEdit, utils::intToStr(patient->getAge()));
    setText(contactEdit, patient->getContact());
    std::string type = patient->getCategory();
    SendMessageA(typeCombo, CB_SETCURSEL, type == "Emergency" ? 1 : (type == "Follow-Up" ? 2 : 0), 0);
    setText(symptomsEdit, "");
    setText(lastVisitEdit, "");
    setText(referEdit, "");
    if (type == "Emergency") {
        SendMessageA(priorityCombo, CB_SETCURSEL, std::atoi(patient->getExtra1().c_str()) - 1, 0);
        setText(symptomsEdit, patient->getExtra2());
    } else if (type == "Follow-Up") {
        setText(lastVisitEdit, patient->getExtra1());
        setText(referEdit, patient->getExtra2());
    }
    fillingForm = false;
    updateTypeFields();
    checkIdField();
    owner->setStatus("Loaded " + patient->getId() + " (" + patient->getName() + ") into the form.");
}

void PatientPanel::registerPatient() {
    try {
        std::string extra1, extra2;
        readExtras(extra1, extra2);
        std::string id = utils::toUpper(utils::trim(getText(idEdit)));
        manager().registerPatient(selectedType(), id, getText(nameEdit), getText(ageEdit),
                                  getText(contactEdit), extra1, extra2);
        owner->refreshAll();
        owner->setStatus("Registered patient " + id + ".");
        showInfo("Patient Registered", "Patient " + id + " (" + utils::trim(getText(nameEdit)) +
                                       ") was registered successfully.\nPatient type: " + selectedType());
        clearForm();
    } catch (const std::exception& error) {
        showException(error);
    } catch (...) {
        showError("Error", "Error: Something unexpected went wrong while adding the patient.");
    }
}

void PatientPanel::updatePatient() {
    try {
        std::string extra1, extra2;
        readExtras(extra1, extra2);
        std::string id = utils::toUpper(utils::trim(getText(idEdit)));
        manager().editPatient(selectedType(), id, getText(nameEdit), getText(ageEdit),
                              getText(contactEdit), extra1, extra2);
        owner->refreshAll();
        owner->setStatus("Updated patient " + id + ".");
        showInfo("Patient Updated", "The details of patient " + id + " were updated.\n"
                                    "Use Undo (Ctrl+Z) to go back to the old details.");
    } catch (const std::exception& error) {
        showException(error);
    } catch (...) {
        showError("Error", "Error: Something unexpected went wrong while updating the patient.");
    }
}

void PatientPanel::deletePatient() {
    try {
        std::string id = utils::toUpper(utils::trim(getText(idEdit)));
        if (id.empty()) {
            showError("Delete Patient", "Error: Select a patient in the table first.");
            return;
        }
        model::PatientRecord* patient = manager().peekPatient(id);
        if (patient == NULL) {
            showError("Record Not Found", "Error: No patient with ID " + id + ".");
            return;
        }
        if (!askYesNo("Delete Patient", "Delete " + patient->getName() + " (" + id + ") and all of their appointments?")) {
            return;
        }
        manager().deletePatient(id);
        owner->refreshAll();
        clearForm();
        owner->setStatus("Deleted patient " + id + ". Press Ctrl+Z to undo.");
        showInfo("Patient Deleted", "Patient " + id + " was deleted.\nYou can undo this with Ctrl+Z.");
    } catch (const std::exception& error) {
        showException(error);
    } catch (...) {
        showError("Error", "Error: Something unexpected went wrong while deleting the patient.");
    }
}

// polymorphism: getDetails() runs the version for the real patient type
void PatientPanel::viewDetails() {
    model::PatientRecord* patient = manager().openPatient(getText(idEdit));
    if (patient == NULL) {
        showError("View Details", "Error: Select a patient in the table first.");
        return;
    }
    showInfo("Patient Details - " + patient->getId(), manager().describePatient(*patient));
}

void PatientPanel::addToQueue() {
    try {
        std::string id = utils::toUpper(utils::trim(getText(idEdit)));
        manager().addToQueue(id);
        owner->refreshAll();
        owner->setStatus(id + " added to the waiting queue.");
        showInfo("Waiting Queue", id + " was added to the waiting queue.\nPosition in queue: " +
                                  utils::intToStr(manager().getQueueSize()));
    } catch (const std::exception& error) {
        showException(error);
    }
}

void PatientPanel::onCommand(int id, int code) {
    if (id == IDC_PAT_TYPE && code == CBN_SELCHANGE) {
        updateTypeFields();
        return;
    }
    if (id == IDC_PAT_ID && code == EN_CHANGE) {
        if (!fillingForm) {
            checkIdField();
        }
        return;
    }
    if (code != BN_CLICKED) {
        return;
    }
    switch (id) {
    case IDC_PAT_NEWID:
        setText(idEdit, manager().suggestPatientId());
        break;
    case IDC_PAT_ADD:
        registerPatient();
        break;
    case IDC_PAT_UPDATE:
        updatePatient();
        break;
    case IDC_PAT_DELETE:
        deletePatient();
        break;
    case IDC_PAT_DETAILS:
        viewDetails();
        break;
    case IDC_PAT_QUEUE:
        addToQueue();
        break;
    case IDC_PAT_CLEAR:
        clearForm();
        break;
    }
}

LRESULT PatientPanel::onNotify(NMHDR* header) {
    if (header->hwndFrom != patientList) {
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

COLORREF PatientPanel::labelColour(HWND control) {
    if (control == idStatus) {
        return idStatusColour;
    }
    return Panel::labelColour(control);
}

}
