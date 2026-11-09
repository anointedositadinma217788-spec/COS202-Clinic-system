#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WinHeaders.h"
#include "../controller/ClinicManager.h"
#include <string>
#include <vector>

namespace gui {

class Panel;
class DashboardPanel;
class SearchPanel;
class QueuePanel;

enum PageNumber {
    PAGE_DASHBOARD = 0,
    PAGE_PATIENTS,
    PAGE_APPOINTMENTS,
    PAGE_SEARCH,
    PAGE_QUEUE,
    PAGE_ADMIN,
    PAGE_COUNT
};

class MainWindow {
public:
    MainWindow(controller::ClinicManager& manager);
    ~MainWindow();

    bool create(HINSTANCE instance);
    void loadAtStartup();
    int run();

    controller::ClinicManager& getManager();
    HWND getHandle() const;
    HFONT getFont() const;
    HFONT getBoldFont() const;
    HFONT getBigFont() const;
    HFONT getMonoFont() const;

    void refreshAll();
    void setStatus(const std::string& text);
    void addAlert(const std::string& text);
    void showPage(int page);

    void saveData();
    void loadData();
    void undoLast();

private:
    controller::ClinicManager& manager;
    HWND window;
    HWND tabs;
    HWND statusBar;
    HACCEL shortcuts;
    HFONT font;
    HFONT boldFont;
    HFONT bigFont;
    HFONT monoFont;

    Panel* pages[PAGE_COUNT];
    DashboardPanel* dashboard;
    SearchPanel* searchPage;
    QueuePanel* queuePage;
    int currentPage;

    std::vector<int> remindedAppointments;
    std::vector<std::string> waitingAlerts;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    HMENU buildMenu();
    void buildShortcuts();
    void resizeChildren();
    void onMenu(int id);
    void onTimer(int id);
    void checkAlerts();
    void updateStatusBar();

    void newData();
    void importPatients();
    void exportPatients();
    bool askToSave();
    std::string chooseFile(bool saving);
};

}

#endif
