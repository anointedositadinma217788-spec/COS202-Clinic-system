#include "MainWindow.h"
#include "Panel.h"
#include "DashboardPanel.h"
#include "PatientPanel.h"
#include "AppointmentPanel.h"
#include "SearchPanel.h"
#include "QueuePanel.h"
#include "AdminPanel.h"
#include "../utils/Validator.h"
#include <ctime>

namespace gui {

enum MenuId {
    ID_FILE_NEW = 1001,
    ID_FILE_LOAD,
    ID_FILE_SAVE,
    ID_FILE_IMPORT,
    ID_FILE_EXPORT,
    ID_FILE_EXIT,
    ID_EDIT_UNDO = 1101,
    ID_EDIT_FIND,
    ID_VIEW_FIRST = 1201,
    ID_HELP_KEYS = 1301,
    ID_HELP_ABOUT
};

static const int TIMER_REFRESH = 1;
static const int TIMER_ALERTS = 2;
static const int WAIT_LIMIT_MINUTES = 15;
static const int REMINDER_MINUTES = 60;

static const char* WINDOW_TITLE = "SCAPMS - Smart Clinic Appointment & Patient Management System";

MainWindow::MainWindow(controller::ClinicManager& manager)
    : manager(manager), window(NULL), tabs(NULL), statusBar(NULL), shortcuts(NULL),
      dashboard(NULL), searchPage(NULL), queuePage(NULL), currentPage(0) {
    for (int i = 0; i < PAGE_COUNT; i++) {
        pages[i] = NULL;
    }
    font = CreateFontA(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    boldFont = CreateFontA(-12, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    bigFont = CreateFontA(-26, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
    monoFont = CreateFontA(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");
}

MainWindow::~MainWindow() {
    for (int i = 0; i < PAGE_COUNT; i++) {
        delete pages[i];
    }
    if (shortcuts != NULL) {
        DestroyAcceleratorTable(shortcuts);
    }
    DeleteObject(font);
    DeleteObject(boldFont);
    DeleteObject(bigFont);
    DeleteObject(monoFont);
}

controller::ClinicManager& MainWindow::getManager() { return manager; }
HWND MainWindow::getHandle() const { return window; }
HFONT MainWindow::getFont() const { return font; }
HFONT MainWindow::getBoldFont() const { return boldFont; }
HFONT MainWindow::getBigFont() const { return bigFont; }
HFONT MainWindow::getMonoFont() const { return monoFont; }

bool MainWindow::create(HINSTANCE instance) {
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = MainWindow::WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "ScapmsMainWindow";
    RegisterClassA(&wc);
    Panel::registerClass(instance);

    // fit the window inside the screen (without the taskbar) and centre it
    RECT screen;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &screen, 0);
    int width = screen.right - screen.left - 20;
    int height = screen.bottom - screen.top - 20;
    if (width > 1180) {
        width = 1180;
    }
    if (height > 760) {
        height = 760;
    }
    int left = screen.left + (screen.right - screen.left - width) / 2;
    int top = screen.top + (screen.bottom - screen.top - height) / 2;

    window = CreateWindowExA(WS_EX_CONTROLPARENT, "ScapmsMainWindow", WINDOW_TITLE,
                             WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                             left, top, width, height,
                             NULL, buildMenu(), instance, this);
    if (window == NULL) {
        return false;
    }

    tabs = CreateWindowExA(0, WC_TABCONTROLA, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP,
                           0, 0, 100, 100, window, NULL, instance, NULL);
    SendMessageA(tabs, WM_SETFONT, (WPARAM)font, TRUE);
    const char* titles[PAGE_COUNT] = {"Dashboard", "Patient Records", "Appointments",
                                      "Search && Sort", "Waiting Queue", "Reports && Admin"};
    for (int i = 0; i < PAGE_COUNT; i++) {
        TCITEMA item;
        ZeroMemory(&item, sizeof(item));
        item.mask = TCIF_TEXT;
        item.pszText = (LPSTR)titles[i];
        SendMessageA(tabs, TCM_INSERTITEMA, i, (LPARAM)&item);
    }

    statusBar = CreateWindowExA(0, STATUSCLASSNAMEA, "", WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                0, 0, 0, 0, window, NULL, instance, NULL);
    SendMessageA(statusBar, WM_SETFONT, (WPARAM)font, TRUE);

    dashboard = new DashboardPanel(this);
    searchPage = new SearchPanel(this);
    queuePage = new QueuePanel(this);
    pages[PAGE_DASHBOARD] = dashboard;
    pages[PAGE_PATIENTS] = new PatientPanel(this);
    pages[PAGE_APPOINTMENTS] = new AppointmentPanel(this);
    pages[PAGE_SEARCH] = searchPage;
    pages[PAGE_QUEUE] = queuePage;
    pages[PAGE_ADMIN] = new AdminPanel(this);
    for (int i = 0; i < PAGE_COUNT; i++) {
        pages[i]->create(window);
    }

    buildShortcuts();
    resizeChildren();
    showPage(PAGE_DASHBOARD);

    SetTimer(window, TIMER_REFRESH, 5000, NULL);
    SetTimer(window, TIMER_ALERTS, 30000, NULL);

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    return true;
}

void MainWindow::loadAtStartup() {
    try {
        int skipped = manager.loadData();
        if (skipped > 0) {
            MessageBoxA(window, ("Some lines in the data files were damaged and were skipped: " +
                                 utils::intToStr(skipped)).c_str(),
                        "Load Warning", MB_OK | MB_ICONWARNING);
        }
        setStatus("Data loaded from " + manager.getDataFolder());
        addAlert("System started. " + utils::intToStr((int)manager.getPatients().size()) +
                 " patients loaded.");
    } catch (const std::exception& error) {
        MessageBoxA(window, (std::string("Error: Could not load the saved data.\n") + error.what()).c_str(),
                    "File Loading Failure", MB_OK | MB_ICONERROR);
        manager.newData();
        setStatus("Started with empty data because the files could not be loaded.");
    }
    refreshAll();
    checkAlerts();
}

int MainWindow::run() {
    MSG message;
    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        if (TranslateAcceleratorA(window, shortcuts, &message)) {
            continue;
        }
        if (IsDialogMessageA(window, &message)) {
            continue;
        }
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return (int)message.wParam;
}

HMENU MainWindow::buildMenu() {
    HMENU bar = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    HMENU editMenu = CreatePopupMenu();
    HMENU viewMenu = CreatePopupMenu();
    HMENU helpMenu = CreatePopupMenu();

    AppendMenuA(fileMenu, MF_STRING, ID_FILE_NEW, "&New\tCtrl+N");
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_LOAD, "&Load\tCtrl+O");
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_SAVE, "&Save\tCtrl+S");
    AppendMenuA(fileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_IMPORT, "&Import Patients (CSV)...\tCtrl+I");
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_EXPORT, "&Export Patients (CSV)...\tCtrl+E");
    AppendMenuA(fileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_EXIT, "E&xit\tCtrl+Q");

    AppendMenuA(editMenu, MF_STRING, ID_EDIT_UNDO, "&Undo Last Admin Operation\tCtrl+Z");
    AppendMenuA(editMenu, MF_STRING, ID_EDIT_FIND, "&Find Patient\tCtrl+F");

    const char* names[PAGE_COUNT] = {"&Dashboard\tCtrl+1", "&Patient Records\tCtrl+2",
                                     "&Appointments\tCtrl+3", "&Search && Sort\tCtrl+4",
                                     "&Waiting Queue\tCtrl+5", "&Reports && Admin\tCtrl+6"};
    for (int i = 0; i < PAGE_COUNT; i++) {
        AppendMenuA(viewMenu, MF_STRING, ID_VIEW_FIRST + i, names[i]);
    }

    AppendMenuA(helpMenu, MF_STRING, ID_HELP_KEYS, "&Keyboard Shortcuts");
    AppendMenuA(helpMenu, MF_STRING, ID_HELP_ABOUT, "&About");

    AppendMenuA(bar, MF_POPUP, (UINT_PTR)fileMenu, "&File");
    AppendMenuA(bar, MF_POPUP, (UINT_PTR)editMenu, "&Edit");
    AppendMenuA(bar, MF_POPUP, (UINT_PTR)viewMenu, "&View");
    AppendMenuA(bar, MF_POPUP, (UINT_PTR)helpMenu, "&Help");
    return bar;
}

void MainWindow::buildShortcuts() {
    ACCEL keys[15];
    BYTE ctrl = FCONTROL | FVIRTKEY;
    keys[0].fVirt = ctrl; keys[0].key = 'N'; keys[0].cmd = ID_FILE_NEW;
    keys[1].fVirt = ctrl; keys[1].key = 'O'; keys[1].cmd = ID_FILE_LOAD;
    keys[2].fVirt = ctrl; keys[2].key = 'S'; keys[2].cmd = ID_FILE_SAVE;
    keys[3].fVirt = ctrl; keys[3].key = 'I'; keys[3].cmd = ID_FILE_IMPORT;
    keys[4].fVirt = ctrl; keys[4].key = 'E'; keys[4].cmd = ID_FILE_EXPORT;
    keys[5].fVirt = ctrl; keys[5].key = 'Q'; keys[5].cmd = ID_FILE_EXIT;
    keys[6].fVirt = ctrl; keys[6].key = 'Z'; keys[6].cmd = ID_EDIT_UNDO;
    keys[7].fVirt = ctrl; keys[7].key = 'F'; keys[7].cmd = ID_EDIT_FIND;
    for (int i = 0; i < PAGE_COUNT; i++) {
        keys[8 + i].fVirt = ctrl;
        keys[8 + i].key = (WORD)('1' + i);
        keys[8 + i].cmd = (WORD)(ID_VIEW_FIRST + i);
    }
    keys[14].fVirt = FVIRTKEY;
    keys[14].key = VK_F1;
    keys[14].cmd = ID_HELP_KEYS;
    shortcuts = CreateAcceleratorTableA(keys, 15);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        CREATESTRUCTA* info = (CREATESTRUCTA*)lParam;
        MainWindow* self = (MainWindow*)info->lpCreateParams;
        self->window = hwnd;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)self);
    }
    MainWindow* self = (MainWindow*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (self != NULL) {
        return self->handleMessage(message, wParam, lParam);
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

LRESULT MainWindow::handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        onMenu(LOWORD(wParam));
        return 0;
    case WM_NOTIFY: {
        NMHDR* header = (NMHDR*)lParam;
        if (header->hwndFrom == tabs && header->code == TCN_SELCHANGE) {
            showPage((int)SendMessageA(tabs, TCM_GETCURSEL, 0, 0));
        }
        break;
    }
    case WM_SIZE:
        resizeChildren();
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* info = (MINMAXINFO*)lParam;
        info->ptMinTrackSize.x = 980;
        info->ptMinTrackSize.y = 620;
        return 0;
    }
    case WM_TIMER:
        onTimer((int)wParam);
        return 0;
    case WM_CLOSE:
        if (askToSave()) {
            DestroyWindow(window);
        }
        return 0;
    case WM_DESTROY:
        KillTimer(window, TIMER_REFRESH);
        KillTimer(window, TIMER_ALERTS);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(window, message, wParam, lParam);
}

void MainWindow::resizeChildren() {
    if (tabs == NULL || statusBar == NULL) {
        return;
    }
    RECT client;
    GetClientRect(window, &client);
    SendMessageA(statusBar, WM_SIZE, 0, 0);
    RECT statusRect;
    GetWindowRect(statusBar, &statusRect);
    int statusHeight = statusRect.bottom - statusRect.top;
    int width = client.right;
    int height = client.bottom - statusHeight;

    int parts[4] = {width - 560, width - 300, width - 150, -1};
    SendMessageA(statusBar, SB_SETPARTS, 4, (LPARAM)parts);

    MoveWindow(tabs, 6, 6, width - 12, height - 10, TRUE);
    RECT area;
    area.left = 6;
    area.top = 6;
    area.right = width - 6;
    area.bottom = height - 4;
    SendMessageA(tabs, TCM_ADJUSTRECT, FALSE, (LPARAM)&area);
    for (int i = 0; i < PAGE_COUNT; i++) {
        if (pages[i] != NULL) {
            pages[i]->setBounds(area.left, area.top, area.right - area.left, area.bottom - area.top);
        }
    }
}

void MainWindow::showPage(int page) {
    if (page < 0 || page >= PAGE_COUNT) {
        return;
    }
    currentPage = page;
    SendMessageA(tabs, TCM_SETCURSEL, page, 0);
    pages[page]->refresh();
    for (int i = 0; i < PAGE_COUNT; i++) {
        pages[i]->setVisible(i == page);
    }
}

void MainWindow::refreshAll() {
    for (int i = 0; i < PAGE_COUNT; i++) {
        pages[i]->refresh();
    }
    updateStatusBar();
}

void MainWindow::setStatus(const std::string& text) {
    SendMessageA(statusBar, SB_SETTEXTA, 0, (LPARAM)text.c_str());
}

void MainWindow::addAlert(const std::string& text) {
    dashboard->addAlert("[" + utils::currentTime() + "]  " + text);
}

void MainWindow::updateStatusBar() {
    std::string counts = "Patients: " + utils::intToStr((int)manager.getPatients().size()) +
                         "    Appointments: " + utils::intToStr((int)manager.getAppointments().size()) +
                         "    Queue: " + utils::intToStr(manager.getQueueSize());
    std::string saved = manager.hasUnsavedChanges() ? "Unsaved changes" : "All changes saved";
    std::string clock = utils::todayDate() + "  " + utils::currentTime();
    SendMessageA(statusBar, SB_SETTEXTA, 1, (LPARAM)counts.c_str());
    SendMessageA(statusBar, SB_SETTEXTA, 2, (LPARAM)saved.c_str());
    SendMessageA(statusBar, SB_SETTEXTA, 3, (LPARAM)clock.c_str());

    std::string title = WINDOW_TITLE;
    if (manager.hasUnsavedChanges()) {
        title += " *";
    }
    SetWindowTextA(window, title.c_str());
}

void MainWindow::onMenu(int id) {
    if (id >= ID_VIEW_FIRST && id < ID_VIEW_FIRST + PAGE_COUNT) {
        showPage(id - ID_VIEW_FIRST);
        return;
    }
    switch (id) {
    case ID_FILE_NEW:
        newData();
        break;
    case ID_FILE_LOAD:
        loadData();
        break;
    case ID_FILE_SAVE:
        saveData();
        break;
    case ID_FILE_IMPORT:
        importPatients();
        break;
    case ID_FILE_EXPORT:
        exportPatients();
        break;
    case ID_FILE_EXIT:
        PostMessageA(window, WM_CLOSE, 0, 0);
        break;
    case ID_EDIT_UNDO:
        undoLast();
        break;
    case ID_EDIT_FIND:
        showPage(PAGE_SEARCH);
        searchPage->focusKeyword();
        break;
    case ID_HELP_KEYS:
        MessageBoxA(window,
                    "Ctrl+N   New (empty) record set\n"
                    "Ctrl+O   Load data from files\n"
                    "Ctrl+S   Save data to files\n"
                    "Ctrl+I   Import patients from CSV\n"
                    "Ctrl+E   Export patients to CSV\n"
                    "Ctrl+Z   Undo last admin operation\n"
                    "Ctrl+F   Find patient (Search page)\n"
                    "Ctrl+Q   Exit\n"
                    "Ctrl+1 to Ctrl+6   Switch between pages\n"
                    "Tab      Move between fields",
                    "Keyboard Shortcuts", MB_OK | MB_ICONINFORMATION);
        break;
    case ID_HELP_ABOUT:
        MessageBoxA(window,
                    "SCAPMS - Smart Clinic Appointment & Patient Management System\n"
                    "COS202 Programming II - Lab Assessment Project\n\n"
                    "Developed by: Anointed Ositadinma\n"
                    "September 2025 cohort - CyberSecurity",
                    "About SCAPMS", MB_OK | MB_ICONINFORMATION);
        break;
    }
}

void MainWindow::onTimer(int id) {
    if (id == TIMER_REFRESH) {
        dashboard->refresh();
        queuePage->refreshWaitingList();
        updateStatusBar();
    } else if (id == TIMER_ALERTS) {
        checkAlerts();
    }
}

// appointment reminders and patients who have waited too long
void MainWindow::checkAlerts() {
    std::vector<model::Appointment*> due = manager.getDueSoon(REMINDER_MINUTES);
    for (size_t i = 0; i < due.size(); i++) {
        bool alreadyDone = false;
        for (size_t j = 0; j < remindedAppointments.size(); j++) {
            if (remindedAppointments[j] == due[i]->getNumber()) {
                alreadyDone = true;
            }
        }
        if (alreadyDone) {
            continue;
        }
        remindedAppointments.push_back(due[i]->getNumber());
        model::PatientRecord* patient = manager.peekPatient(due[i]->getPatientId());
        std::string name = patient != NULL ? patient->getName() : due[i]->getPatientId();
        std::string text = "Reminder: " + name + " sees " + due[i]->getDoctor() + " at " +
                           due[i]->getTime() + " (appointment #" + utils::intToStr(due[i]->getNumber()) + ")";
        addAlert(text);
        setStatus(text);
        MessageBeep(MB_ICONASTERISK);
    }

    std::vector<controller::QueueEntry> queue = manager.getQueue();
    std::vector<std::string> stillWaiting;
    time_t now = time(NULL);
    for (size_t i = 0; i < queue.size(); i++) {
        bool alreadyDone = false;
        for (size_t j = 0; j < waitingAlerts.size(); j++) {
            if (waitingAlerts[j] == queue[i].patientId) {
                alreadyDone = true;
            }
        }
        int minutes = (int)((now - queue[i].joinedAt) / 60);
        if (alreadyDone || minutes >= WAIT_LIMIT_MINUTES) {
            stillWaiting.push_back(queue[i].patientId);
        }
        if (!alreadyDone && minutes >= WAIT_LIMIT_MINUTES) {
            model::PatientRecord* patient = manager.peekPatient(queue[i].patientId);
            std::string name = patient != NULL ? patient->getName() : queue[i].patientId;
            std::string text = "Waiting alert: " + name + " has been in the queue for " +
                               utils::intToStr(minutes) + " minutes";
            addAlert(text);
            setStatus(text);
            MessageBeep(MB_ICONEXCLAMATION);
        }
    }
    waitingAlerts = stillWaiting;
}

// ---------- file menu ----------

bool MainWindow::askToSave() {
    if (!manager.hasUnsavedChanges()) {
        return true;
    }
    int answer = MessageBoxA(window, "You have unsaved changes. Save them before closing?",
                             "Unsaved Changes", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (answer == IDCANCEL) {
        return false;
    }
    if (answer == IDYES) {
        try {
            manager.saveData();
        } catch (const std::exception& error) {
            MessageBoxA(window, (std::string("Error: ") + error.what()).c_str(), "Save Failed",
                        MB_OK | MB_ICONERROR);
            return false;
        }
    }
    return true;
}

void MainWindow::saveData() {
    try {
        manager.saveData();
        updateStatusBar();
        setStatus("All data saved to " + manager.getDataFolder());
        MessageBoxA(window, "Patients, appointments and the waiting queue were saved.",
                    "Saved", MB_OK | MB_ICONINFORMATION);
    } catch (const std::exception& error) {
        MessageBoxA(window, (std::string("Error: ") + error.what()).c_str(), "Save Failed",
                    MB_OK | MB_ICONERROR);
    }
}

void MainWindow::loadData() {
    if (manager.hasUnsavedChanges()) {
        int answer = MessageBoxA(window, "Loading will throw away your unsaved changes. Continue?",
                                 "Load Data", MB_YESNO | MB_ICONQUESTION);
        if (answer != IDYES) {
            return;
        }
    }
    try {
        int skipped = manager.loadData();
        remindedAppointments.clear();
        waitingAlerts.clear();
        refreshAll();
        setStatus("Data loaded from " + manager.getDataFolder());
        std::string message = utils::intToStr((int)manager.getPatients().size()) + " patients and " +
                              utils::intToStr((int)manager.getAppointments().size()) +
                              " appointments loaded.";
        if (skipped > 0) {
            message += "\n" + utils::intToStr(skipped) + " damaged line(s) were skipped.";
        }
        MessageBoxA(window, message.c_str(), "Data Loaded", MB_OK | MB_ICONINFORMATION);
        checkAlerts();
    } catch (const std::exception& error) {
        manager.newData();
        refreshAll();
        MessageBoxA(window, (std::string("Error: ") + error.what()).c_str(), "File Loading Failure",
                    MB_OK | MB_ICONERROR);
    }
}

void MainWindow::newData() {
    int answer = MessageBoxA(window,
                             "Start a new, empty set of records?\n"
                             "The files on disk are not changed until you save.",
                             "New", MB_YESNO | MB_ICONQUESTION);
    if (answer != IDYES) {
        return;
    }
    manager.newData();
    remindedAppointments.clear();
    waitingAlerts.clear();
    refreshAll();
    setStatus("Started a new empty record set.");
}

void MainWindow::undoLast() {
    try {
        std::string what;
        if (manager.undoLast(what)) {
            refreshAll();
            setStatus("Undone: " + what);
            MessageBoxA(window, ("Undone: " + what).c_str(), "Undo", MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBoxA(window, "There is no admin operation left to undo.", "Undo",
                        MB_OK | MB_ICONINFORMATION);
        }
    } catch (const std::exception& error) {
        refreshAll();
        MessageBoxA(window, (std::string("Error: Undo failed.\n") + error.what()).c_str(), "Undo",
                    MB_OK | MB_ICONERROR);
    }
}

std::string MainWindow::chooseFile(bool saving) {
    char fileName[MAX_PATH];
    lstrcpyA(fileName, saving ? "patients_export.csv" : "");
    OPENFILENAMEA dialog;
    ZeroMemory(&dialog, sizeof(dialog));
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = window;
    dialog.lpstrFilter = "CSV Files (*.csv)\0*.csv\0Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = "csv";
    BOOL chosen;
    if (saving) {
        dialog.lpstrTitle = "Export Patients";
        dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        chosen = GetSaveFileNameA(&dialog);
    } else {
        dialog.lpstrTitle = "Import Patients";
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        chosen = GetOpenFileNameA(&dialog);
    }
    if (!chosen) {
        return "";
    }
    return fileName;
}

void MainWindow::importPatients() {
    std::string path = chooseFile(false);
    if (path.empty()) {
        return;
    }
    try {
        int skipped = 0;
        int added = manager.importPatients(path, skipped);
        refreshAll();
        std::string message = utils::intToStr(added) + " patient(s) imported.";
        if (skipped > 0) {
            message += "\n" + utils::intToStr(skipped) + " row(s) skipped (duplicate ID or invalid data).";
        }
        setStatus("Imported patients from " + path);
        MessageBoxA(window, message.c_str(), "Import Finished", MB_OK | MB_ICONINFORMATION);
    } catch (const std::exception& error) {
        MessageBoxA(window, (std::string("Error: ") + error.what()).c_str(), "Import Failed",
                    MB_OK | MB_ICONERROR);
    }
}

void MainWindow::exportPatients() {
    std::string path = chooseFile(true);
    if (path.empty()) {
        return;
    }
    try {
        manager.exportPatients(path);
        setStatus("Exported patients to " + path);
        MessageBoxA(window, ("Patient records exported to:\n" + path).c_str(), "Export Finished",
                    MB_OK | MB_ICONINFORMATION);
    } catch (const std::exception& error) {
        MessageBoxA(window, (std::string("Error: ") + error.what()).c_str(), "Export Failed",
                    MB_OK | MB_ICONERROR);
    }
}

}
