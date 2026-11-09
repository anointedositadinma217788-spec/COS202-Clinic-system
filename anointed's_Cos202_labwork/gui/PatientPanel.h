#ifndef PATIENTPANEL_H
#define PATIENTPANEL_H

#include "Panel.h"

namespace gui {

class PatientPanel : public Panel {
public:
    PatientPanel(MainWindow* owner);
    void refresh();

protected:
    void buildControls();
    void onCommand(int id, int code);
    LRESULT onNotify(NMHDR* header);
    COLORREF labelColour(HWND control);

private:
    HWND idEdit;
    HWND nameEdit;
    HWND ageEdit;
    HWND contactEdit;
    HWND typeCombo;

    // fields that only show for some patient types
    HWND priorityLabel;
    HWND priorityCombo;
    HWND symptomsLabel;
    HWND symptomsEdit;
    HWND lastVisitLabel;
    HWND lastVisitEdit;
    HWND referLabel;
    HWND referEdit;

    HWND idStatus;
    COLORREF idStatusColour;
    HWND patientList;
    bool fillingForm;

    std::string selectedType() const;
    void readExtras(std::string& extra1, std::string& extra2) const;
    void updateTypeFields();
    void checkIdField();
    void clearForm();
    void loadSelected();

    void registerPatient();
    void updatePatient();
    void deletePatient();
    void viewDetails();
    void addToQueue();
};

}

#endif
