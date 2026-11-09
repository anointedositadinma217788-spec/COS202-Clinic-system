#include "DashboardPanel.h"
#include "MainWindow.h"
#include "../controller/ReportGenerator.h"
#include "../utils/Validator.h"

namespace gui {

enum DashboardId {
    IDC_DASH_SAVE = 100,
    IDC_DASH_LOAD,
    IDC_DASH_REFRESH,
    IDC_DASH_TODAY,
    IDC_DASH_ALERTS
};

static const char* CARD_TITLES[8] = {
    "Total Patients", "General Patients", "Emergency Patients", "Follow-Up Patients",
    "Appointments Today", "Pending Appointments", "Completed Visits", "Waiting in Queue"
};

DashboardPanel::DashboardPanel(MainWindow* owner) : Panel(owner) {}

void DashboardPanel::buildControls() {
    BoxLayout* root = new BoxLayout(true, 10, 14);

    HWND title = makeLabel("Clinic Dashboard");
    SendMessageA(title, WM_SETFONT, (WPARAM)owner->getBigFont(), TRUE);
    clockLabel = makeLabel("");
    SendMessageA(clockLabel, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    BoxLayout* top = new BoxLayout(false);
    top->addWidget(title, 300, 1);
    top->addWidget(clockLabel, 330);
    root->addLayout(top, 36);

    // eight small cards in a 4 x 2 grid
    GridLayout* cards = new GridLayout(4, 74, 10);
    for (int i = 0; i < CARD_COUNT; i++) {
        HWND box = makeGroup(CARD_TITLES[i]);
        cardValues[i] = makeControl("STATIC", "0", SS_CENTER | SS_CENTERIMAGE, 0, 0);
        SendMessageA(cardValues[i], WM_SETFONT, (WPARAM)owner->getBigFont(), TRUE);
        BoxLayout* inside = new BoxLayout(true);
        inside->addWidget(cardValues[i], 30, 1);
        cards->addLayout(new GroupLayout(box, inside), i / 4, i % 4);
    }
    root->addLayout(cards, 158);

    BoxLayout* buttons = new BoxLayout(false, 8);
    buttons->addWidget(makeButton(IDC_DASH_SAVE, "Save Data", "Save patients, appointments and queue to the data folder (Ctrl+S)"), 120);
    buttons->addWidget(makeButton(IDC_DASH_LOAD, "Load Data", "Reload everything from the data files (Ctrl+O)"), 120);
    buttons->addWidget(makeButton(IDC_DASH_REFRESH, "Refresh Now", "Refresh the numbers now (they also refresh every 5 seconds)"), 120);
    buttons->addSpace(10, 1);
    buttons->addWidget(makeLabel("Statistics refresh automatically every 5 seconds."), 300);
    root->addLayout(buttons, 30);

    BoxLayout* bottom = new BoxLayout(false, 12);
    BoxLayout* leftSide = new BoxLayout(true, 4);
    todayLabel = makeLabel("Today's Appointments");
    SendMessageA(todayLabel, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    leftSide->addWidget(todayLabel, 22);
    todayList = makeListView(IDC_DASH_TODAY);
    addColumn(todayList, 0, "Time", 60);
    addColumn(todayList, 1, "No.", 45);
    addColumn(todayList, 2, "Patient", 140);
    addColumn(todayList, 3, "Doctor", 95);
    addColumn(todayList, 4, "Service", 140);
    addColumn(todayList, 5, "Status", 80);
    leftSide->addWidget(todayList, 100, 1);
    bottom->addLayout(leftSide, 300, 3);

    BoxLayout* rightSide = new BoxLayout(true, 4);
    HWND alertTitle = makeLabel("Alerts && Reminders (timer)");
    SendMessageA(alertTitle, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    rightSide->addWidget(alertTitle, 22);
    alertList = makeListBox(IDC_DASH_ALERTS);
    addTip(alertList, "Appointment reminders and waiting-time alerts appear here");
    rightSide->addWidget(alertList, 100, 1);
    bottom->addLayout(rightSide, 300, 2);
    root->addLayout(bottom, 100, 1);

    layout = root;
}

void DashboardPanel::refresh() {
    controller::ClinicManager& clinic = manager();
    controller::ReportGenerator reports(clinic.getDatabase());
    std::string today = utils::todayDate();

    const std::vector<model::Appointment*>& appts = clinic.getAppointments();
    int pending = 0;
    int completed = 0;
    for (size_t i = 0; i < appts.size(); i++) {
        if (appts[i]->isPending()) {
            pending++;
        } else if (appts[i]->getStatus() == "Completed") {
            completed++;
        }
    }
    std::vector<model::Appointment*> todayAppts = clinic.getAppointmentsOn(today);

    int values[CARD_COUNT];
    values[0] = (int)clinic.getPatients().size();
    values[1] = reports.countCategory("General");
    values[2] = reports.countCategory("Emergency");
    values[3] = reports.countCategory("Follow-Up");
    values[4] = (int)todayAppts.size();
    values[5] = pending;
    values[6] = completed;
    values[7] = clinic.getQueueSize();
    for (int i = 0; i < CARD_COUNT; i++) {
        setText(cardValues[i], utils::intToStr(values[i]));
    }
    setText(clockLabel, "Today: " + today + "    Time: " + utils::currentTime());
    setText(todayLabel, "Today's Appointments (" + today + ")");

    // sort today's list by time with a small insertion sort
    for (int i = 1; i < (int)todayAppts.size(); i++) {
        model::Appointment* current = todayAppts[i];
        int j = i - 1;
        while (j >= 0 && todayAppts[j]->getTime() > current->getTime()) {
            todayAppts[j + 1] = todayAppts[j];
            j--;
        }
        todayAppts[j + 1] = current;
    }

    clearList(todayList);
    for (size_t i = 0; i < todayAppts.size(); i++) {
        model::PatientRecord* patient = clinic.peekPatient(todayAppts[i]->getPatientId());
        std::vector<std::string> cells;
        cells.push_back(todayAppts[i]->getTime());
        cells.push_back("#" + utils::intToStr(todayAppts[i]->getNumber()));
        cells.push_back(patient != NULL ? patient->getName() : todayAppts[i]->getPatientId());
        cells.push_back(todayAppts[i]->getDoctor());
        cells.push_back(todayAppts[i]->getService());
        cells.push_back(todayAppts[i]->getStatus());
        addRow(todayList, cells, colourFor(todayAppts[i]->getStatus()));
    }
}

void DashboardPanel::addAlert(const std::string& text) {
    SendMessageA(alertList, LB_INSERTSTRING, 0, (LPARAM)text.c_str());
}

void DashboardPanel::onCommand(int id, int code) {
    if (code != BN_CLICKED) {
        return;
    }
    if (id == IDC_DASH_SAVE) {
        owner->saveData();
    } else if (id == IDC_DASH_LOAD) {
        owner->loadData();
    } else if (id == IDC_DASH_REFRESH) {
        owner->refreshAll();
        owner->setStatus("Dashboard refreshed.");
    }
}

LRESULT DashboardPanel::onNotify(NMHDR* header) {
    if (header->hwndFrom == todayList && header->code == NM_CUSTOMDRAW) {
        return paintRow(header);
    }
    return 0;
}

COLORREF DashboardPanel::labelColour(HWND control) {
    for (int i = 0; i < CARD_COUNT; i++) {
        if (control == cardValues[i]) {
            if (i == 2) {
                return RGB(190, 30, 30);
            }
            if (i == 3) {
                return RGB(30, 90, 170);
            }
            if (i == 7) {
                return RGB(200, 120, 0);
            }
            return RGB(20, 110, 80);
        }
    }
    return Panel::labelColour(control);
}

}
