#include "QueuePanel.h"
#include "MainWindow.h"
#include "../utils/Validator.h"
#include <ctime>

namespace gui {

enum QueueId {
    IDC_QUE_PATIENT = 100,
    IDC_QUE_ADD,
    IDC_QUE_NEXT,
    IDC_QUE_LIST
};

QueuePanel::QueuePanel(MainWindow* owner) : Panel(owner) {}

void QueuePanel::buildControls() {
    BoxLayout* root = new BoxLayout(true, 8, 12);

    HWND box = makeGroup("Waiting Queue - first come, first served");
    BoxLayout* row = new BoxLayout(false, 8);
    row->addWidget(makeLabel("Patient:"), 55);
    patientCombo = makeCombo(IDC_QUE_PATIENT, true);
    addTip(patientCombo, "Choose a patient or type their ID");
    row->addWidget(patientCombo, 300);
    row->addWidget(makeButton(IDC_QUE_ADD, "Add to Queue", "Put the patient at the back of the queue"), 120);
    row->addWidget(makeButton(IDC_QUE_NEXT, "Call Next Patient", "Serve the patient at the front of the queue"), 140);
    row->addSpace(10, 1);
    root->addLayout(new GroupLayout(box, row), 62);

    countLabel = makeLabel("");
    SendMessageA(countLabel, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    root->addWidget(countLabel, 20);

    waitingList = makeListView(IDC_QUE_LIST);
    addColumn(waitingList, 0, "Position", 65);
    addColumn(waitingList, 1, "Patient ID", 80);
    addColumn(waitingList, 2, "Name", 180);
    addColumn(waitingList, 3, "Category", 90);
    addColumn(waitingList, 4, "Priority", 95);
    addColumn(waitingList, 5, "Joined At", 75);
    addColumn(waitingList, 6, "Waiting Time", 100);
    root->addWidget(waitingList, 100, 1);

    servingLabel = makeLabel("Now serving: nobody yet");
    SendMessageA(servingLabel, WM_SETFONT, (WPARAM)owner->getBigFont(), TRUE);
    root->addWidget(servingLabel, 40);

    root->addWidget(makeLabel("Yellow rows have waited 15 minutes or more. The waiting times update every 5 seconds "
                              "and an alert is shown on the dashboard."), 20);
    layout = root;
}

void QueuePanel::refresh() {
    std::string typed = getText(patientCombo);
    SendMessageA(patientCombo, CB_RESETCONTENT, 0, 0);
    const std::vector<model::PatientRecord*>& patients = manager().getPatients();
    for (size_t i = 0; i < patients.size(); i++) {
        addComboItem(patientCombo, patients[i]->getId() + " - " + patients[i]->getName());
    }
    setText(patientCombo, typed);
    refreshWaitingList();
}

// called by the timer so the waiting minutes stay correct
void QueuePanel::refreshWaitingList() {
    std::vector<controller::QueueEntry> queue = manager().getQueue();
    time_t now = time(NULL);
    clearList(waitingList);
    for (size_t i = 0; i < queue.size(); i++) {
        model::PatientRecord* patient = manager().peekPatient(queue[i].patientId);
        int minutes = (int)((now - queue[i].joinedAt) / 60);
        std::vector<std::string> cells;
        cells.push_back(utils::intToStr((int)i + 1));
        cells.push_back(queue[i].patientId);
        cells.push_back(patient != NULL ? patient->getName() : "(unknown)");
        cells.push_back(patient != NULL ? patient->getCategory() : "-");
        cells.push_back(patient != NULL ? patient->getPriorityLabel() : "-");
        cells.push_back(utils::clockOf(queue[i].joinedAt));
        cells.push_back(utils::intToStr(minutes) + " min");
        int colour = ROW_NORMAL;
        if (minutes >= 15) {
            colour = ROW_YELLOW;
        } else if (patient != NULL) {
            colour = colourFor(patient->getCategory());
        }
        addRow(waitingList, cells, colour);
    }
    setText(countLabel, "Patients waiting: " + utils::intToStr((int)queue.size()));
}

void QueuePanel::addPatient() {
    try {
        std::string text = utils::trim(getText(patientCombo));
        std::string id = text;
        size_t dash = text.find(" - ");
        if (dash != std::string::npos) {
            id = text.substr(0, dash);
        }
        manager().addToQueue(id);
        setText(patientCombo, "");
        owner->refreshAll();
        owner->setStatus(utils::toUpper(id) + " joined the waiting queue.");
    } catch (const std::exception& error) {
        showException(error);
    }
}

void QueuePanel::callNext() {
    controller::QueueEntry served;
    if (!manager().callNextPatient(served)) {
        showInfo("Waiting Queue", "The waiting queue is empty.");
        return;
    }
    model::PatientRecord* patient = manager().openPatient(served.patientId);
    int minutes = (int)((time(NULL) - served.joinedAt) / 60);
    std::string name = patient != NULL ? patient->getName() : served.patientId;
    setText(servingLabel, "Now serving: " + served.patientId + " - " + name);
    owner->refreshAll();
    owner->setStatus("Now serving " + name + ".");
    showInfo("Next Patient", "Now serving: " + name + " (" + served.patientId + ")\n" +
                             "Waited " + utils::intToStr(minutes) + " minute(s).");
}

void QueuePanel::onCommand(int id, int code) {
    if (code != BN_CLICKED) {
        return;
    }
    if (id == IDC_QUE_ADD) {
        addPatient();
    } else if (id == IDC_QUE_NEXT) {
        callNext();
    }
}

LRESULT QueuePanel::onNotify(NMHDR* header) {
    if (header->hwndFrom == waitingList && header->code == NM_CUSTOMDRAW) {
        return paintRow(header);
    }
    return 0;
}

COLORREF QueuePanel::labelColour(HWND control) {
    if (control == servingLabel) {
        return RGB(20, 110, 80);
    }
    return Panel::labelColour(control);
}

}
