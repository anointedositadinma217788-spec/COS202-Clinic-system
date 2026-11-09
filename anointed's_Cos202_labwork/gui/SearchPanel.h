#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include "Panel.h"

namespace gui {

class SearchPanel : public Panel {
public:
    SearchPanel(MainWindow* owner);
    void refresh();
    void focusKeyword();

protected:
    void buildControls();
    void onCommand(int id, int code);
    LRESULT onNotify(NMHDR* header);
    COLORREF labelColour(HWND control);

private:
    HWND fieldCombo;
    HWND methodCombo;
    HWND keywordEdit;
    HWND searchHint;
    HWND sortAlgorithmCombo;
    HWND sortFieldCombo;
    HWND explainLabel;
    HWND sortState;
    HWND resultTitle;
    HWND resultList;
    HWND recentList;

    void updateHint();
    void doSearch();
    void doSort();
    void showResults(const std::vector<model::PatientRecord*>& patients);
    void refreshRecent();
};

}

#endif
