#ifndef ADMINPANEL_H
#define ADMINPANEL_H

#include "Panel.h"

namespace gui {

// reports plus the undo stack for admin operations
class AdminPanel : public Panel {
public:
    AdminPanel(MainWindow* owner);
    void refresh();

protected:
    void buildControls();
    void onCommand(int id, int code);

private:
    HWND dateEdit;
    HWND reportText;
    HWND undoLabel;
    HWND undoList;

    void showReport(int which);
};

}

#endif
