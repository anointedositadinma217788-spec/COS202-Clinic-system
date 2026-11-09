#ifndef QUEUEPANEL_H
#define QUEUEPANEL_H

#include "Panel.h"

namespace gui {

class QueuePanel : public Panel {
public:
    QueuePanel(MainWindow* owner);
    void refresh();
    void refreshWaitingList();

protected:
    void buildControls();
    void onCommand(int id, int code);
    LRESULT onNotify(NMHDR* header);
    COLORREF labelColour(HWND control);

private:
    HWND patientCombo;
    HWND waitingList;
    HWND countLabel;
    HWND servingLabel;

    void addPatient();
    void callNext();
};

}

#endif
