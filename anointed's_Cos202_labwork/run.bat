@echo off
setlocal
cd /d "%~dp0"

echo ========================================
echo Clinic Management System
echo Developer: Anointed Ositadinma
echo ========================================
echo.

if not exist bin mkdir bin

echo Building application...
g++ -O1 -mwindows -o bin\clinic_app.exe main.cpp gui\Win32Gui.cpp model\PatientRecord.cpp model\GeneralPatient.cpp model\EmergencyPatient.cpp model\FollowUpPatient.cpp model\Appointment.cpp model\ClinicDatabase.cpp -lcomctl32 -lgdi32 2> build_errors.txt

if %errorlevel% neq 0 (
    echo Build failed. See build_errors.txt
    pause
    exit /b 1
)

echo Build successful. Starting application. Close the app or this window to exit.
start "" bin\clinic_app.exe
exit /b 0
