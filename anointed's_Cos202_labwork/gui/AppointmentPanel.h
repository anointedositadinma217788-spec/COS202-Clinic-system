#ifndef APPOINTMENTPANEL_H
#define APPOINTMENTPANEL_H

#include "Panel.h"

namespace gui {

class AppointmentPanel : public Panel {
public:
    AppointmentPanel(MainWindow* owner);
    void refresh();

protected:
    void buildControls();
    void onCommand(int id, int code);
    LRESULT onNotify(NMHDR* header);
    COLORREF labelColour(HWND control);

private:
    HWND patientEdit;
    HWND patientInfo;
    COLORREF patientInfoColour;
    HWND doctorCombo;
    HWND dateEdit;
    HWND timeCombo;
    HWND serviceCombo;
    HWND numberEdit;
    HWND filterCombo;
    HWND apptList;

    void showPatientInfo();
    void schedule();
    void clearForm();
    void changeAppointment(int action);
    void loadSelected();
};

}

#endif
