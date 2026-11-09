#include "AdminPanel.h"
#include "MainWindow.h"
#include "../controller/ReportGenerator.h"
#include "../utils/Validator.h"

namespace gui {

enum AdminId {
    IDC_ADM_REPORT1 = 100,
    IDC_ADM_REPORT2,
    IDC_ADM_REPORT3,
    IDC_ADM_REPORT4,
    IDC_ADM_REPORT5,
    IDC_ADM_DATE,
    IDC_ADM_UNDO,
    IDC_ADM_UNDOLIST,
    IDC_ADM_TEXT
};

AdminPanel::AdminPanel(MainWindow* owner) : Panel(owner) {}

void AdminPanel::buildControls() {
    BoxLayout* root = new BoxLayout(false, 12, 12);
    BoxLayout* left = new BoxLayout(true, 10);

    HWND reportBox = makeGroup("Reports");
    BoxLayout* reports = new BoxLayout(true, 6);
    reports->addWidget(makeButton(IDC_ADM_REPORT1, "1. Most Frequently Attended", "Patients with the most completed appointments"), 28);
    reports->addWidget(makeButton(IDC_ADM_REPORT2, "2. Pending Appointments", "Patients who still have scheduled appointments"), 28);
    reports->addWidget(makeButton(IDC_ADM_REPORT3, "3. Category Distribution", "How many General, Emergency and Follow-Up patients"), 28);
    BoxLayout* dateRow = new BoxLayout(false, 6);
    dateRow->addWidget(makeLabel("Date:"), 35);
    dateEdit = makeEdit(IDC_ADM_DATE);
    addTip(dateEdit, "Date for the daily summary, YYYY-MM-DD");
    dateRow->addWidget(dateEdit, 100, 1);
    reports->addLayout(dateRow, 24);
    reports->addWidget(makeButton(IDC_ADM_REPORT4, "4. Daily Appointment Summary", "Totals for the date above"), 28);
    reports->addWidget(makeButton(IDC_ADM_REPORT5, "5. Consultation Fees", "Fee for each patient, worked out by patient type"), 28);
    left->addLayout(new GroupLayout(reportBox, reports), 245);

    HWND undoBox = makeGroup("Undo Last Administrative Operation");
    BoxLayout* undo = new BoxLayout(true, 6);
    undo->addWidget(makeButton(IDC_ADM_UNDO, "Undo Last Operation  (Ctrl+Z)", "Reverse the operation at the top of the undo stack"), 30);
    undoLabel = makeLabel("");
    undo->addWidget(undoLabel, 20);
    undoList = makeListBox(IDC_ADM_UNDOLIST);
    addTip(undoList, "The undo stack - the top line is undone first");
    undo->addWidget(undoList, 60, 1);
    left->addLayout(new GroupLayout(undoBox, undo), 100, 1);
    root->addLayout(left, 290);

    BoxLayout* right = new BoxLayout(true, 4);
    HWND outputTitle = makeLabel("Report Output");
    SendMessageA(outputTitle, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    right->addWidget(outputTitle, 20);
    reportText = makeControl("EDIT", "", WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY |
                             ES_AUTOVSCROLL | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, IDC_ADM_TEXT);
    SendMessageA(reportText, WM_SETFONT, (WPARAM)owner->getMonoFont(), TRUE);
    right->addWidget(reportText, 100, 1);
    root->addLayout(right, 300, 1);

    layout = root;
    setText(dateEdit, utils::todayDate());
    setText(reportText, "Choose a report on the left.");
}

void AdminPanel::refresh() {
    std::vector<std::string> history = manager().getUndoHistory();
    SendMessageA(undoList, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < history.size(); i++) {
        std::string line = (i == 0 ? "TOP >  " : "       ") + history[i];
        SendMessageA(undoList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }
    if (history.empty()) {
        SendMessageA(undoList, LB_ADDSTRING, 0, (LPARAM)"(nothing to undo)");
    }
    setText(undoLabel, "Operations on the stack: " + utils::intToStr(manager().getUndoCount()));
}

void AdminPanel::showReport(int which) {
    try {
        controller::ReportGenerator reports(manager().getDatabase());
        std::string text;
        if (which == IDC_ADM_REPORT1) {
            text = reports.mostAttended();
        } else if (which == IDC_ADM_REPORT2) {
            text = reports.pendingAppointments();
        } else if (which == IDC_ADM_REPORT3) {
            text = reports.categoryDistribution();
        } else if (which == IDC_ADM_REPORT4) {
            text = reports.dailySummary(utils::trim(getText(dateEdit)));
        } else {
            text = reports.feeSummary();
        }
        setText(reportText, windowsText(text));
        owner->setStatus("Report generated.");
    } catch (const std::exception& error) {
        showException(error);
    }
}

void AdminPanel::onCommand(int id, int code) {
    if (code != BN_CLICKED) {
        return;
    }
    if (id >= IDC_ADM_REPORT1 && id <= IDC_ADM_REPORT5) {
        showReport(id);
    } else if (id == IDC_ADM_UNDO) {
        owner->undoLast();
    }
}

}
