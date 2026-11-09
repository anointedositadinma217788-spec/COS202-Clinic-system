#ifndef DASHBOARDPANEL_H
#define DASHBOARDPANEL_H

#include "Panel.h"

namespace gui {

class DashboardPanel : public Panel {
public:
    DashboardPanel(MainWindow* owner);
    void refresh();
    void addAlert(const std::string& text);

protected:
    void buildControls();
    void onCommand(int id, int code);
    LRESULT onNotify(NMHDR* header);
    COLORREF labelColour(HWND control);

private:
    static const int CARD_COUNT = 8;
    HWND cardValues[CARD_COUNT];
    HWND clockLabel;
    HWND todayLabel;
    HWND todayList;
    HWND alertList;
};

}

#endif
