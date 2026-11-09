#ifndef PANEL_H
#define PANEL_H

#include "WinHeaders.h"
#include "Layout.h"
#include "../controller/ClinicManager.h"
#include <string>
#include <vector>
#include <exception>

namespace gui {

class MainWindow;

// background colours for list view rows
enum RowColour {
    ROW_NORMAL = 0,
    ROW_RED,
    ROW_BLUE,
    ROW_YELLOW,
    ROW_GREEN,
    ROW_GREY
};

// base class for every tab page in the main window
class Panel {
public:
    Panel(MainWindow* owner);
    virtual ~Panel();

    static void registerClass(HINSTANCE instance);

    bool create(HWND parent);
    HWND getHandle() const;
    void setBounds(int x, int y, int width, int height);
    void setVisible(bool visible);
    void relayout();

    virtual void refresh() = 0;

protected:
    HWND handle;
    HWND tooltip;
    MainWindow* owner;
    Layout* layout;

    virtual void buildControls() = 0;
    virtual void onCommand(int id, int code) = 0;
    virtual LRESULT onNotify(NMHDR* header);
    virtual COLORREF labelColour(HWND control);

    controller::ClinicManager& manager();

    HWND makeControl(const char* className, const std::string& text, int style, int exStyle, int id);
    HWND makeLabel(const std::string& text, bool multiLine = false);
    HWND makeEdit(int id, int style = 0);
    HWND makeButton(int id, const std::string& text, const std::string& tip);
    HWND makeCombo(int id, bool editable);
    HWND makeGroup(const std::string& text);
    HWND makeListView(int id);
    HWND makeListBox(int id);
    void addTip(HWND control, const std::string& text);

    void addComboItem(HWND combo, const std::string& text);
    int comboIndex(HWND combo) const;
    std::string getText(HWND control) const;
    void setText(HWND control, const std::string& text);
    std::string windowsText(const std::string& text) const;

    void addColumn(HWND list, int index, const std::string& title, int width);
    void addRow(HWND list, const std::vector<std::string>& cells, int colour);
    void clearList(HWND list);
    int selectedRow(HWND list) const;
    std::string cellText(HWND list, int row, int column) const;
    LRESULT paintRow(NMHDR* header);
    int colourFor(const std::string& categoryOrStatus) const;

    void showInfo(const std::string& title, const std::string& message);
    void showError(const std::string& title, const std::string& message);
    void showException(const std::exception& error);
    bool askYesNo(const std::string& title, const std::string& question);

private:
    static LRESULT CALLBACK PanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

}

#endif
