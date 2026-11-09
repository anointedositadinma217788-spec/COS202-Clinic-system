#include "Win32Gui.h"
#include "../model/ClinicDatabase.h"
#include "../model/ClinicExceptions.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include "../model/Appointment.h"
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>

using namespace clinic_mgmt;

static ClinicDatabase db;

#define IDC_TAB        100
#define IDC_READY      101
#define IDC_REG        200
#define IDC_NAME       201
#define IDC_AGE        202
#define IDC_PHONE      203
#define IDC_TYPE       204
#define IDC_F1         205
#define IDC_F2         206
#define IDC_LBL_F1     207
#define IDC_LBL_F2     208
#define IDC_BTN_REG    209
#define IDC_BTN_UPD    210
#define IDC_BTN_DEL    211
#define IDC_BTN_CLR    212
#define IDC_P_LIST     213
#define IDC_A_REG      300
#define IDC_A_NO       301
#define IDC_A_DOCTOR   302
#define IDC_A_TIME     303
#define IDC_A_SERVICE  304
#define IDC_BTN_SCHED  305
#define IDC_BTN_CANCA  306
#define IDC_A_LIST     307
#define IDC_S_REG      400
#define IDC_S_NAME     401
#define IDC_S_ALGO     402
#define IDC_BTN_LIN    403
#define IDC_BTN_BIN    404
#define IDC_BTN_LINN   405
#define IDC_BTN_SORT   406
#define IDC_S_LIST     407
#define IDC_Q_REG      500
#define IDC_BTN_QADD   501
#define IDC_BTN_QNEXT  502
#define IDC_Q_LIST     503
#define IDC_BTN_RP1    600
#define IDC_BTN_RP2    601
#define IDC_BTN_RP3    602
#define IDC_RPT_TEXT   603
#define IDM_SAVE       900
#define IDM_EXIT       901
#define IDM_UNDO       902
#define IDM_ABOUT      903

#define PANEL_PATIENTS 0
#define PANEL_APPTS    1
#define PANEL_SEARCH   2
#define PANEL_QUEUE    3
#define PANEL_REPORTS  4
#define PANEL_COUNT    5

static ClinicDatabase* gdb;
static HWND gMain, gTabs, gStatus;
static HWND gPanels[PANEL_COUNT];
static HFONT gFont;

static HWND gReg, gName, gAge, gPhone, gType, gF1, gF2, gLblF1, gLblF2, gPList;
static HWND gAReg, gANo, gADoctor, gATime, gAService, gAList;
static HWND gSReg, gSName, gSAlgo, gSList;
static HWND gQReg, gQList;
static HWND gRptText;

static std::vector<std::string> gPatientRegs;
static std::vector<int> gApptNums;

static const char* appHint = "";

static HWND makeControl(HWND parent, const char* cls, const char* text, int exstyle,
                        int style, int id, int x, int y, int w, int h) {
    HWND control = CreateWindowExA(exstyle, cls, text, WS_CHILD | WS_VISIBLE | style,
                                   x, y, w, h, parent, (HMENU)id, GetModuleHandleA(NULL), NULL);
    if (gFont) {
        SendMessageA(control, WM_SETFONT, (WPARAM)gFont, TRUE);
    }
    return control;
}

static std::string editText(HWND control) {
    char buffer[256];
    int len = GetWindowTextA(control, buffer, sizeof(buffer));
    return std::string(buffer, len);
}

static std::string comboSelectText(HWND combo) {
    int index = (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);
    if (index == CB_ERR) {
        return "";
    }
    char buffer[128];
    SendMessageA(combo, CB_GETLBTEXT, index, (LPARAM)buffer);
    return std::string(buffer);
}

static void refreshPatients() {
    SendMessageA(gPList, LB_RESETCONTENT, 0, 0);
    gPatientRegs.clear();
    std::vector<PatientRecord*> all = gdb->getAllPatients();
    for (size_t i = 0; i < all.size(); i++) {
        std::stringstream lineStream;
        lineStream << all[i]->getRegNumber() << "  |  " << all[i]->getFullName()
                   << "  |  " << all[i]->getPatientAge() << "  |  "
                   << all[i]->getPhoneNumber() << "  |  " << all[i]->getCategory();
        SendMessageA(gPList, LB_ADDSTRING, 0, (LPARAM)lineStream.str().c_str());
        gPatientRegs.push_back(all[i]->getRegNumber());
    }
}

static void refreshAppointments() {
    SendMessageA(gAList, LB_RESETCONTENT, 0, 0);
    gApptNums.clear();
    std::vector<Appointment*> appts = gdb->getAllAppointments();
    for (size_t i = 0; i < appts.size(); i++) {
        std::stringstream lineStream;
        lineStream << "#" << appts[i]->getApptNumber() << "  |  "
                   << appts[i]->getPatientReg() << "  |  "
                   << appts[i]->getDoctor() << "  |  "
                   << appts[i]->getAppointmentTime() << "  |  "
                   << appts[i]->getServiceType() << "  |  "
                   << appts[i]->getApptStatus();
        SendMessageA(gAList, LB_ADDSTRING, 0, (LPARAM)lineStream.str().c_str());
        gApptNums.push_back(appts[i]->getApptNumber());
    }
}

static void refreshQueue() {
    SendMessageA(gQList, LB_RESETCONTENT, 0, 0);
    std::vector<std::string> queue = gdb->queueSnapshot();
    for (size_t i = 0; i < queue.size(); i++) {
        std::stringstream lineStream;
        lineStream << (i + 1) << ".  " << queue[i];
        PatientRecord* patient = gdb->findPatientByReg(queue[i]);
        if (patient) {
            lineStream << "  -  " << patient->getFullName();
        }
        SendMessageA(gQList, LB_ADDSTRING, 0, (LPARAM)lineStream.str().c_str());
    }
}

static void updateStatus(const char* hint) {
    std::vector<PatientRecord*> patients = gdb->getAllPatients();
    std::vector<Appointment*> appts = gdb->getAllAppointments();
    std::stringstream parts[3];
    parts[0] << hint;
    parts[1] << "Patients: " << patients.size();
    parts[2] << "Appointments: " << appts.size() << "   Queue: " << gdb->queueSize();
    SendMessageA(gStatus, SB_SETTEXTA, 0, (LPARAM)parts[0].str().c_str());
    SendMessageA(gStatus, SB_SETTEXTA, 1, (LPARAM)parts[1].str().c_str());
    SendMessageA(gStatus, SB_SETTEXTA, 2, (LPARAM)parts[2].str().c_str());
}

static void box(HWND parent, const char* title, const char* msg, int type) {
    MessageBoxA(parent, msg, title, type);
}

static void showCaughtError(HWND parent, const std::exception& err) {
    box(parent, "Input Error", err.what(), MB_ICONERROR);
}

static void updateExtraLabels() {
    std::string type = comboSelectText(gType);
    if (type == "Emergency") {
        SetWindowTextA(gLblF1, "Priority (1-5)");
        SetWindowTextA(gLblF2, "Symptoms");
    } else if (type == "FollowUp") {
        SetWindowTextA(gLblF1, "Last Visit Date");
        SetWindowTextA(gLblF2, "Referring Doctor");
    } else {
        SetWindowTextA(gLblF1, "Extra Info 1 (not used)");
        SetWindowTextA(gLblF2, "Extra Info 2 (not used)");
        SetWindowTextA(gF1, "");
        SetWindowTextA(gF2, "");
    }
}

static void doRegister(HWND parent) {
    try {
        std::string reg = editText(gReg);
        std::string name = editText(gName);
        int age = std::atoi(editText(gAge).c_str());
        std::string phone = editText(gPhone);
        std::string type = comboSelectText(gType);

        PatientRecord* patient = nullptr;
        if (type == "Emergency") {
            int priority = std::atoi(editText(gF1).c_str());
            if (priority < 1 || priority > 5) {
                throw InvalidInputException("Priority must be between 1 and 5.");
            }
            patient = new EmergencyPatient(reg, name, age, phone,
                                           priority, editText(gF2));
        } else if (type == "FollowUp") {
            patient = new FollowUpPatient(reg, name, age, phone,
                                          editText(gF1), editText(gF2));
        } else {
            patient = new GeneralPatient(reg, name, age, phone);
        }
        try {
            gdb->addPatient(patient);
        } catch (...) {
            delete patient;
            throw;
        }
        refreshPatients();
        updateStatus(appHint);
        box(parent, "Done", "Patient registered successfully.", MB_OK | MB_ICONINFORMATION);
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doEdit(HWND parent) {
    try {
        std::string reg = editText(gReg);
        std::string name = editText(gName);
        int age = std::atoi(editText(gAge).c_str());
        std::string phone = editText(gPhone);
        if (gdb->editPatient(reg, name, age, phone)) {
            refreshPatients();
            updateStatus(appHint);
            box(parent, "Done", "Patient details updated.", MB_OK | MB_ICONINFORMATION);
        } else {
            box(parent, "Not Found", ("No patient with reg number " + reg).c_str(),
                MB_ICONWARNING);
        }
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doDelete(HWND parent) {
    std::string reg = editText(gReg);
    if (reg.empty()) {
        box(parent, "Input Error", "Enter the reg number to delete.", MB_ICONERROR);
        return;
    }
    std::string question = "Delete patient " + reg + "?";
    if (MessageBoxA(parent, question.c_str(), "Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        if (gdb->removePatient(reg)) {
            refreshPatients();
            updateStatus(appHint);
            box(parent, "Done", "Patient deleted.", MB_OK | MB_ICONINFORMATION);
        } else {
            box(parent, "Not Found", ("No patient with reg number " + reg).c_str(),
                MB_ICONWARNING);
        }
    }
}

static void doLoadSelected(HWND parent) {
    int index = (int)SendMessageA(gPList, LB_GETCURSEL, 0, 0);
    if (index == LB_ERR || index >= (int)gPatientRegs.size()) {
        return;
    }
    PatientRecord* patient = gdb->findPatientByReg(gPatientRegs[index]);
    if (!patient) {
        return;
    }
    SetWindowTextA(gReg, patient->getRegNumber().c_str());
    SetWindowTextA(gName, patient->getFullName().c_str());
    std::stringstream ageStream;
    ageStream << patient->getPatientAge();
    SetWindowTextA(gAge, ageStream.str().c_str());
    SetWindowTextA(gPhone, patient->getPhoneNumber().c_str());
    std::string type = patient->getCategory();
    SendMessageA(gType, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)type.c_str());
    updateExtraLabels();
    if (type == "Emergency") {
        EmergencyPatient* ep = dynamic_cast<EmergencyPatient*>(patient);
        std::stringstream pStream;
        pStream << ep->getPriorityLevel();
        SetWindowTextA(gF1, pStream.str().c_str());
        SetWindowTextA(gF2, ep->getComplaint().c_str());
    } else if (type == "FollowUp") {
        FollowUpPatient* fp = dynamic_cast<FollowUpPatient*>(patient);
        SetWindowTextA(gF1, fp->getLastVisit().c_str());
        SetWindowTextA(gF2, fp->getDoctorName().c_str());
    }
    box(parent, "Selected", ("Editing " + type + " patient " + patient->getRegNumber()).c_str(),
        MB_OK | MB_ICONINFORMATION);
}

static void doSchedule(HWND parent) {
    try {
        gdb->addAppointment(editText(gAReg), comboSelectText(gADoctor),
                            editText(gATime), editText(gAService));
        refreshAppointments();
        updateStatus(appHint);
        box(parent, "Done", "Appointment scheduled.", MB_OK | MB_ICONINFORMATION);
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doCancelAppt(HWND parent) {
    int apptNum = std::atoi(editText(gANo).c_str());
    if (gdb->cancelAppointment(apptNum)) {
        refreshAppointments();
        updateStatus(appHint);
        box(parent, "Done", "Appointment cancelled.", MB_OK | MB_ICONINFORMATION);
    } else {
        box(parent, "Not Found", "No appointment with that number.", MB_ICONWARNING);
    }
}

static void doUndo(HWND parent) {
    if (gdb->undoLastAction()) {
        refreshPatients();
        refreshAppointments();
        refreshQueue();
        updateStatus(appHint);
    } else {
        box(parent, "Undo", "Nothing left to undo.", MB_OK | MB_ICONINFORMATION);
    }
}

static void doSaveNow(HWND parent) {
    try {
        gdb->saveAll();
        box(parent, "Done", "All records saved to file.", MB_OK | MB_ICONINFORMATION);
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doSearch(HWND parent, int kind) {
    try {
        std::vector<PatientRecord*> results;
        std::string header = "";

        if (kind == 1) {
            std::string name = editText(gSName);
            if (name.empty()) {
                throw InvalidInputException("Type a name to search for.");
            }
            results = gdb->linearSearchByName(name);
            header = "Linear search (name): ";
        } else if (kind == 2) {
            std::string reg = editText(gSReg);
            if (reg.empty()) {
                throw InvalidInputException("Type a reg number to search for.");
            }
            PatientRecord* found = gdb->binarySearchByReg(reg);
            if (found) {
                results.push_back(found);
            }
            header = "Binary search (reg number): ";
        } else {
            std::string reg = editText(gSReg);
            if (reg.empty()) {
                throw InvalidInputException("Type a reg number to search for.");
            }
            PatientRecord* found = gdb->findPatientByReg(reg);
            if (found) {
                results.push_back(found);
            }
            header = "Linear search (reg number): ";
        }

        SendMessageA(gSList, LB_RESETCONTENT, 0, 0);
        std::string line = header + std::string(results.size() == 1 ? "1 match" : "matches") +
                           " = " + std::to_string((long long)results.size());
        SendMessageA(gSList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        for (size_t i = 0; i < results.size(); i++) {
            std::stringstream lineStream;
            lineStream << "  " << results[i]->getRegNumber() << "  |  "
                       << results[i]->getFullName() << "  |  "
                       << results[i]->getPatientAge() << "  |  "
                       << results[i]->getCategory();
            SendMessageA(gSList, LB_ADDSTRING, 0, (LPARAM)lineStream.str().c_str());
        }
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doSort(HWND parent) {
    try {
        std::vector<PatientRecord*> sorted = gdb->getAllPatients();
        SendMessageA(gSList, LB_RESETCONTENT, 0, 0);
        if (sorted.empty()) {
            throw InvalidInputException("There are no patients to sort.");
        }
        int algo = (int)SendMessageA(gSAlgo, CB_GETCURSEL, 0, 0);
        if (algo == 0) {
            gdb->selectionSortByAgePublic(sorted);
        } else {
            gdb->insertionSortByName(sorted);
        }
        SendMessageA(gSList, LB_ADDSTRING, 0,
                     (LPARAM)(algo == 0 ? "Selection sort by age:" : "Insertion sort by name:"));
        for (size_t i = 0; i < sorted.size(); i++) {
            std::stringstream lineStream;
            lineStream << "  " << (i + 1) << ".  " << sorted[i]->getRegNumber() << "  |  "
                       << sorted[i]->getFullName() << "  |  "
                       << sorted[i]->getPatientAge() << "  |  "
                       << sorted[i]->getCategory();
            SendMessageA(gSList, LB_ADDSTRING, 0, (LPARAM)lineStream.str().c_str());
        }
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doQueueAdd(HWND parent) {
    try {
        gdb->addToQueue(editText(gQReg));
        refreshQueue();
        updateStatus(appHint);
    } catch (const std::exception& err) {
        showCaughtError(parent, err);
    }
}

static void doQueueNext(HWND parent) {
    std::string next;
    if (gdb->callNextPatient(next)) {
        refreshQueue();
        updateStatus(appHint);
        box(parent, "Next Patient", ("Now attending:\n" + next).c_str(),
            MB_OK | MB_ICONINFORMATION);
    } else {
        box(parent, "Queue Empty", "The waiting queue is empty.", MB_ICONINFORMATION);
    }
}

static std::string buildPatientReport() {
    std::vector<PatientRecord*> all = gdb->getAllPatients();
    std::stringstream text;
    text << "REPORT 1: PATIENT SUMMARY\n";
    text << "===========================\n\n";
    text << "Total registered patients: " << all.size() << "\n\n";
    text << "General:    " << gdb->recursiveCountByCategory("General") << "\n";
    text << "Emergency:  " << gdb->recursiveCountByCategory("Emergency") << "\n";
    text << "Follow-Up:  " << gdb->recursiveCountByCategory("FollowUp") << "\n";
    text << "  (counts computed with a recursive function)\n\n";
    if (!all.empty()) {
        int ageTotal = 0;
        for (size_t i = 0; i < all.size(); i++) {
            ageTotal += all[i]->getPatientAge();
        }
        text << "Average age: " << ageTotal / (int)all.size() << " years\n\n";
        text << "Registered patients:\n";
        for (size_t i = 0; i < all.size(); i++) {
            text << "  " << all[i]->getRegNumber() << "  " << all[i]->getFullName()
                 << "  (" << all[i]->getCategory() << ")\n";
        }
    }
    return text.str();
}

static std::string buildAppointmentReport() {
    std::vector<Appointment*> appts = gdb->getAllAppointments();
    std::stringstream text;
    text << "REPORT 2: APPOINTMENT SUMMARY\n";
    text << "================================\n\n";
    text << "Total appointments booked: " << appts.size() << "\n\n";
    int scheduled = 0, cancelled = 0, completed = 0;
    for (size_t i = 0; i < appts.size(); i++) {
        std::string status = appts[i]->getApptStatus();
        if (status == "Scheduled") {
            scheduled++;
        } else if (status == "Cancelled") {
            cancelled++;
        } else if (status == "Completed") {
            completed++;
        }
    }
    text << "Scheduled:  " << scheduled << "\n";
    text << "Cancelled:  " << cancelled << "\n";
    text << "Completed:  " << completed << "\n\n";
    text << "Appointments:\n";
    for (size_t i = 0; i < appts.size(); i++) {
        text << "  #" << appts[i]->getApptNumber() << "  " << appts[i]->getPatientReg()
             << "  with " << appts[i]->getDoctor() << "  (" << appts[i]->getApptStatus() << ")\n";
    }
    return text.str();
}

static std::string buildQueueReport() {
    std::stringstream text;
    text << "REPORT 3: WAITING QUEUE\n";
    text << "=========================\n\n";
    text << "Patients waiting: " << gdb->queueSize() << "\n\n";
    std::vector<std::string> queue = gdb->queueSnapshot();
    int position = 1;
    for (size_t i = 0; i < queue.size(); i++) {
        text << "  " << position++ << ". " << queue[i];
        PatientRecord* patient = gdb->findPatientByReg(queue[i]);
        if (patient) {
            text << "  -  " << patient->getFullName();
        }
        text << "\n";
    }
    if (queue.size() == 0) {
        text << "  (the queue is currently empty)\n";
    }
    text << "\nThe queue is managed with std::queue (first in, first out).\n";
    return text.str();
}

static void doReport(HWND parent, std::string title, std::string body) {
    SetWindowTextA(gRptText, body.c_str());
    box(parent, title.c_str(), "Report generated in the text area.", MB_OK | MB_ICONINFORMATION);
}

static LRESULT CALLBACK ClinicWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int note = HIWORD(wParam);
        if (id == IDC_TYPE && note == CBN_SELCHANGE) {
            updateExtraLabels();
            return DefWindowProcA(hwnd, message, wParam, lParam);
        }
        if (id == IDC_P_LIST && note == LBN_DBLCLK) {
            doLoadSelected(hwnd);
            return 0;
        }
        switch (id) {
        case IDM_SAVE:  doSaveNow(hwnd);   return 0;
        case IDM_EXIT:  DestroyWindow(hwnd); return 0;
        case IDM_UNDO:  doUndo(hwnd);      return 0;
        case IDM_ABOUT:
            box(hwnd, "About", "Clinic Management System\nCOS202 Lab Project\nAnointed Ositadinma",
                MB_OK | MB_ICONINFORMATION);
            return 0;
        case IDC_BTN_REG:    doRegister(hwnd); return 0;
        case IDC_BTN_UPD:    doEdit(hwnd);     return 0;
        case IDC_BTN_DEL:    doDelete(hwnd);   return 0;
        case IDC_BTN_CLR:
            SetWindowTextA(gReg, "");  SetWindowTextA(gName, "");
            SetWindowTextA(gAge, "");  SetWindowTextA(gPhone, "");
            SetWindowTextA(gF1, "");   SetWindowTextA(gF2, "");
            return 0;
        case IDC_BTN_SCHED:  doSchedule(hwnd);  return 0;
        case IDC_BTN_CANCA:  doCancelAppt(hwnd); return 0;
        case IDC_BTN_LIN:    doSearch(hwnd, 0);  return 0;
        case IDC_BTN_BIN:    doSearch(hwnd, 2);  return 0;
        case IDC_BTN_LINN:   doSearch(hwnd, 1);  return 0;
        case IDC_BTN_SORT:   doSort(hwnd);       return 0;
        case IDC_BTN_QADD:   doQueueAdd(hwnd);   return 0;
        case IDC_BTN_QNEXT:  doQueueNext(hwnd);  return 0;
        case IDC_BTN_RP1:    doReport(hwnd, "Report 1", buildPatientReport()); return 0;
        case IDC_BTN_RP2:    doReport(hwnd, "Report 2", buildAppointmentReport()); return 0;
        case IDC_BTN_RP3:    doReport(hwnd, "Report 3", buildQueueReport()); return 0;
        }
        break;
    }
    case WM_NOTIFY: {
        NMHDR* notice = (NMHDR*)lParam;
        if (notice->idFrom == IDC_TAB && notice->code == TCN_SELCHANGE) {
            int page = (int)SendMessageA(gTabs, TCM_GETCURSEL, 0, 0);
            for (int i = 0; i < PANEL_COUNT; i++) {
                ShowWindow(gPanels[i], (i == page) ? SW_SHOW : SW_HIDE);
            }
        }
        break;
    }
    case WM_SIZE:
        SendMessageA(gStatus, WM_SIZE, 0, 0);
        return DefWindowProcA(hwnd, message, wParam, lParam);
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

static HMENU buildMenu() {
    HMENU menuBar = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    HMENU editMenu = CreatePopupMenu();
    HMENU helpMenu = CreatePopupMenu();
    AppendMenuA(fileMenu, MF_STRING, IDM_SAVE, "Save Now");
    AppendMenuA(fileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(fileMenu, MF_STRING, IDM_EXIT, "Exit");
    AppendMenuA(editMenu, MF_STRING, IDM_UNDO, "Undo Last Action");
    AppendMenuA(helpMenu, MF_STRING, IDM_ABOUT, "About");
    AppendMenuA(menuBar, MF_POPUP, (UINT_PTR)fileMenu, "File");
    AppendMenuA(menuBar, MF_POPUP, (UINT_PTR)editMenu, "Edit");
    AppendMenuA(menuBar, MF_POPUP, (UINT_PTR)helpMenu, "Help");
    return menuBar;
}

static void buildPatientsPanel(HWND panel) {
    makeControl(panel, "BUTTON", "Reg No:", 0, BS_GROUPBOX, 0, 5, 8, 340, 205);
    makeControl(panel, "STATIC", "Reg Number:", 0, 0, 0, 20, 22, 105, 16);
    gReg = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_REG, 130, 20, 205, 20);
    makeControl(panel, "STATIC", "Full Name:", 0, 0, 0, 20, 47, 105, 16);
    gName = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_NAME, 130, 45, 205, 20);
    makeControl(panel, "STATIC", "Age:", 0, 0, 0, 20, 72, 105, 16);
    gAge = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_AGE, 130, 70, 205, 20);
    makeControl(panel, "STATIC", "Phone:", 0, 0, 0, 20, 97, 105, 16);
    gPhone = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_PHONE, 130, 95, 205, 20);
    makeControl(panel, "STATIC", "Patient Type:", 0, 0, 0, 20, 122, 105, 16);
    gType = makeControl(panel, "COMBOBOX", "", 0, WS_TABSTOP | CBS_DROPDOWNLIST, IDC_TYPE,
                        130, 120, 205, 100);
    SendMessageA(gType, CB_ADDSTRING, 0, (LPARAM)"General");
    SendMessageA(gType, CB_ADDSTRING, 0, (LPARAM)"Emergency");
    SendMessageA(gType, CB_ADDSTRING, 0, (LPARAM)"FollowUp");
    SendMessageA(gType, CB_SETCURSEL, 0, 0);
    gLblF1 = makeControl(panel, "STATIC", "Extra Info 1 (not used)", 0, 0, 0, 20, 147, 120, 16);
    gF1 = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_F1, 130, 145, 205, 20);
    gLblF2 = makeControl(panel, "STATIC", "Extra Info 2 (not used)", 0, 0, 0, 360, 22, 120, 16);
    gF2 = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_F2, 360, 45, 205, 20);

    makeControl(panel, "BUTTON", "Register Patient", 0, BS_PUSHBUTTON, IDC_BTN_REG, 370, 75, 130, 26);
    makeControl(panel, "BUTTON", "Update Details", 0, BS_PUSHBUTTON, IDC_BTN_UPD, 370, 107, 130, 26);
    makeControl(panel, "BUTTON", "Delete Patient", 0, BS_PUSHBUTTON, IDC_BTN_DEL, 370, 139, 130, 26);
    makeControl(panel, "BUTTON", "Clear Form", 0, BS_PUSHBUTTON, IDC_BTN_CLR, 370, 171, 130, 26);

    makeControl(panel, "STATIC",
                "All patients (double-click one to load it back into this form for editing):",
                0, 0, 0, 10, 225, 640, 16);
    gPList = makeControl(panel, "LISTBOX", "", WS_EX_CLIENTEDGE,
                         WS_VSCROLL | LBS_NOTIFY, IDC_P_LIST, 10, 244, 880, 230);
}

static void buildAppointmentsPanel(HWND panel) {
    makeControl(panel, "BUTTON", "Schedule", 0, BS_GROUPBOX, 0, 5, 8, 340, 165);
    makeControl(panel, "STATIC", "Patient Reg:", 0, 0, 0, 20, 22, 105, 16);
    gAReg = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_A_REG, 130, 20, 205, 20);
    makeControl(panel, "STATIC", "Doctor:", 0, 0, 0, 20, 47, 105, 16);
    gADoctor = makeControl(panel, "COMBOBOX", "", 0, WS_TABSTOP | CBS_DROPDOWNLIST,
                          IDC_A_DOCTOR, 130, 45, 205, 150);
    for (int i = 0; i < gdb->doctorCount_count(); i++) {
        SendMessageA(gADoctor, CB_ADDSTRING, 0, (LPARAM)gdb->getDoctor(i).c_str());
    }
    SendMessageA(gADoctor, CB_SETCURSEL, 0, 0);
    makeControl(panel, "STATIC", "Date/Time:", 0, 0, 0, 20, 72, 105, 16);
    gATime = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_A_TIME, 130, 70, 205, 20);
    makeControl(panel, "STATIC", "Service:", 0, 0, 0, 20, 97, 105, 16);
    gAService = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_A_SERVICE, 130, 95, 205, 20);
    makeControl(panel, "BUTTON", "Schedule Appointment", 0, BS_PUSHBUTTON, IDC_BTN_SCHED, 360, 20, 160, 28);
    makeControl(panel, "BUTTON", "Cancel Appt (No:)", 0, BS_PUSHBUTTON, IDC_BTN_CANCA, 360, 56, 160, 28);
    makeControl(panel, "STATIC", "Appt No:", 0, 0, 0, 360, 90, 60, 16);
    gANo = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_A_NO, 425, 87, 95, 20);

    makeControl(panel, "STATIC", "All booked appointments:", 0, 0, 0, 10, 185, 300, 16);
    gAList = makeControl(panel, "LISTBOX", "", WS_EX_CLIENTEDGE,
                         WS_VSCROLL | LBS_NOTIFY, IDC_A_LIST, 10, 204, 880, 250);
}

static void buildSearchPanel(HWND panel) {
    makeControl(panel, "STATIC", "Reg Number:", 0, 0, 0, 20, 20, 105, 16);
    gSReg = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_S_REG, 130, 18, 190, 20);
    makeControl(panel, "BUTTON", "Linear Search", 0, BS_PUSHBUTTON, IDC_BTN_LIN, 340, 15, 130, 26);
    makeControl(panel, "BUTTON", "Binary Search", 0, BS_PUSHBUTTON, IDC_BTN_BIN, 480, 15, 130, 26);

    makeControl(panel, "STATIC", "Name (or part):", 0, 0, 0, 20, 50, 105, 16);
    gSName = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_S_NAME, 130, 48, 190, 20);
    makeControl(panel, "BUTTON", "Linear Search", 0, BS_PUSHBUTTON, IDC_BTN_LINN, 340, 45, 130, 26);

    gSAlgo = makeControl(panel, "COMBOBOX", "", 0, WS_TABSTOP | CBS_DROPDOWNLIST,
                         IDC_S_ALGO, 130, 78, 220, 100);
    SendMessageA(gSAlgo, CB_ADDSTRING, 0, (LPARAM)"Selection Sort - by Age");
    SendMessageA(gSAlgo, CB_ADDSTRING, 0, (LPARAM)"Insertion Sort - by Name");
    SendMessageA(gSAlgo, CB_SETCURSEL, 0, 0);
    makeControl(panel, "BUTTON", "Run Sort", 0, BS_PUSHBUTTON, IDC_BTN_SORT, 360, 75, 130, 26);

    makeControl(panel, "STATIC", "Results:", 0, 0, 0, 10, 115, 200, 16);
    gSList = makeControl(panel, "LISTBOX", "", WS_EX_CLIENTEDGE,
                         WS_VSCROLL | LBS_NOTIFY, IDC_S_LIST, 10, 134, 880, 320);
}

static void buildQueuePanel(HWND panel) {
    makeControl(panel, "STATIC", "Patient Reg:", 0, 0, 0, 20, 18, 105, 16);
    gQReg = makeControl(panel, "EDIT", "", 0, WS_BORDER | ES_AUTOHSCROLL, IDC_Q_REG, 130, 15, 190, 20);
    makeControl(panel, "BUTTON", "Add to Queue", 0, BS_PUSHBUTTON, IDC_BTN_QADD, 340, 12, 130, 26);
    makeControl(panel, "BUTTON", "Call Next Patient", 0, BS_PUSHBUTTON, IDC_BTN_QNEXT, 480, 12, 140, 26);

    makeControl(panel, "STATIC", "Current waiting queue (first patient is calling order 1):",
                0, 0, 0, 10, 55, 400, 16);
    gQList = makeControl(panel, "LISTBOX", "", WS_EX_CLIENTEDGE,
                         WS_VSCROLL | LBS_NOTIFY, IDC_Q_LIST, 10, 74, 380, 380);
}

static void buildReportsPanel(HWND panel) {
    makeControl(panel, "BUTTON", "Report 1: Patients", 0, BS_PUSHBUTTON, IDC_BTN_RP1, 20, 12, 150, 26);
    makeControl(panel, "BUTTON", "Report 2: Appointments", 0, BS_PUSHBUTTON, IDC_BTN_RP2, 180, 12, 170, 26);
    makeControl(panel, "BUTTON", "Report 3: Queue", 0, BS_PUSHBUTTON, IDC_BTN_RP3, 360, 12, 140, 26);
    gRptText = makeControl(panel, "EDIT", "", WS_EX_CLIENTEDGE,
                           WS_VSCROLL | ES_MULTILINE | ES_READONLY, IDC_RPT_TEXT, 10, 45, 880, 450);
}

int runClinicApp(HINSTANCE instance) {
    gdb = &db;

    InitCommonControls();

    gFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = ClinicWndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "ClinicAppWindow";
    RegisterClassA(&wc);

    gMain = CreateWindowExA(0, "ClinicAppWindow",
                            "Clinic Management System - COS202 Lab Project",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT, CW_USEDEFAULT, 940, 660,
                            NULL, buildMenu(), instance, NULL);

    gTabs = makeControl(gMain, WC_TABCONTROLA, "", 0, WS_VISIBLE, IDC_TAB, 2, 0, 920, 30);

    for (int i = 0; i < PANEL_COUNT; i++) {
        gPanels[i] = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_CLIPSIBLINGS,
                                     8, 32, 910, 560, gMain, NULL, instance, NULL);
    }

    buildPatientsPanel(gPanels[PANEL_PATIENTS]);
    buildAppointmentsPanel(gPanels[PANEL_APPTS]);
    buildSearchPanel(gPanels[PANEL_SEARCH]);
    buildQueuePanel(gPanels[PANEL_QUEUE]);
    buildReportsPanel(gPanels[PANEL_REPORTS]);

    TCITEM tie;
    ZeroMemory(&tie, sizeof(tie));
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPSTR)"Patients";
    SendMessageA(gTabs, TCM_INSERTITEMA, PANEL_PATIENTS, (LPARAM)&tie);
    tie.pszText = (LPSTR)"Appointments";
    SendMessageA(gTabs, TCM_INSERTITEMA, PANEL_APPTS, (LPARAM)&tie);
    tie.pszText = (LPSTR)"Search & Sort";
    SendMessageA(gTabs, TCM_INSERTITEMA, PANEL_SEARCH, (LPARAM)&tie);
    tie.pszText = (LPSTR)"Waiting Queue";
    SendMessageA(gTabs, TCM_INSERTITEMA, PANEL_QUEUE, (LPARAM)&tie);
    tie.pszText = (LPSTR)"Reports";
    SendMessageA(gTabs, TCM_INSERTITEMA, PANEL_REPORTS, (LPARAM)&tie);

    for (int i = 1; i < PANEL_COUNT; i++) {
        ShowWindow(gPanels[i], SW_HIDE);
    }

    gStatus = CreateWindowExA(0, STATUSCLASSNAMEA, "Ready",
                              WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                              0, 0, 0, 24, gMain, (HMENU)IDC_READY, instance, NULL);
    int statusParts[2] = {600, 760};
    SendMessageA(gStatus, SB_SETPARTS, 2, (LPARAM)statusParts);

    try {
        db.loadAll();
        updateStatus("Data loaded from files.");
    } catch (const std::exception& err) {
        box(gMain, "File Warning", ("Could not load saved data.\n" + std::string(err.what())).c_str(),
            MB_ICONWARNING);
        updateStatus("Started without saved data (file problem).");
    }
    refreshPatients();
    refreshAppointments();
    refreshQueue();
    updateExtraLabels();

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
