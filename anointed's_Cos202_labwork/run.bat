@echo off
setlocal
cd /d "%~dp0"

echo ==========================================================
echo  SCAPMS - Smart Clinic Appointment and Patient Management
echo  COS202 Lab Project - Anointed Ositadinma
echo ==========================================================
echo.

tasklist /FI "IMAGENAME eq SCAPMS.exe" 2>nul | find /I "SCAPMS.exe" >nul
if %errorlevel% equ 0 (
    echo SCAPMS is already open. Close it first if you want to rebuild.
    pause
    exit /b 0
)

where g++ >nul 2>&1
if %errorlevel% neq 0 (
    if exist SCAPMS.exe (
        echo g++ was not found, starting the program that is already built...
        start "" SCAPMS.exe
        exit /b 0
    )
    echo ERROR: g++ was not found. Install MinGW and add its bin folder to PATH.
    pause
    exit /b 1
)

if not exist build mkdir build
if not exist data mkdir data

echo Building...
windres resources\app.rc -O coff -o build\app.res
if %errorlevel% neq 0 goto failed

g++ -std=c++11 -O2 -mwindows -static-libgcc -static-libstdc++ -o SCAPMS.exe ^
    main.cpp ^
    model\PatientRecord.cpp model\GeneralPatient.cpp model\EmergencyPatient.cpp ^
    model\FollowUpPatient.cpp model\PatientFactory.cpp model\Appointment.cpp ^
    model\ClinicDatabase.cpp ^
    controller\ClinicManager.cpp controller\AppointmentController.cpp ^
    controller\SearchEngine.cpp controller\SortEngine.cpp controller\ReportGenerator.cpp ^
    utils\Validator.cpp utils\IDGenerator.cpp utils\FileHandler.cpp ^
    gui\Layout.cpp gui\Panel.cpp gui\MainWindow.cpp gui\DashboardPanel.cpp ^
    gui\PatientPanel.cpp gui\AppointmentPanel.cpp gui\SearchPanel.cpp ^
    gui\QueuePanel.cpp gui\AdminPanel.cpp ^
    build\app.res -lcomctl32 -lcomdlg32 -lgdi32 2> build\errors.txt
if %errorlevel% neq 0 goto failed

echo Build finished. Starting SCAPMS...
start "" SCAPMS.exe
exit /b 0

:failed
echo.
echo Build failed. The errors are below and in build\errors.txt
type build\errors.txt
pause
exit /b 1
