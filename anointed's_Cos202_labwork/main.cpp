// COS202 Programming II - Lab Assessment Project
// Smart Clinic Appointment & Patient Management System (SCAPMS)
// Name: Anointed Ositadinma
// Class: September 2025 cohort - CyberSecurity

#include "gui/WinHeaders.h"
#include "gui/MainWindow.h"
#include "controller/ClinicManager.h"
#include <string>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int) {
    INITCOMMONCONTROLSEX controls;
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);

    // data folder sits next to the exe so it works however the app is started
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string folder = exePath;
    size_t slash = folder.find_last_of("\\/");
    if (slash != std::string::npos) {
        folder = folder.substr(0, slash);
    }

    controller::ClinicManager manager;
    manager.setDataFolder(folder + "\\data");

    gui::MainWindow mainWindow(manager);
    if (!mainWindow.create(instance)) {
        MessageBoxA(NULL, "The main window could not be created.", "SCAPMS", MB_OK | MB_ICONERROR);
        return 1;
    }
    mainWindow.loadAtStartup();
    return mainWindow.run();
}
