#include "Panel.h"
#include "MainWindow.h"
#include "../model/ClinicExceptions.h"

namespace gui {

static const char* PANEL_CLASS = "ScapmsPanel";

Panel::Panel(MainWindow* owner) : handle(NULL), tooltip(NULL), owner(owner), layout(NULL) {}

Panel::~Panel() {
    delete layout;
}

void Panel::registerClass(HINSTANCE instance) {
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = Panel::PanelProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = PANEL_CLASS;
    RegisterClassA(&wc);
}

bool Panel::create(HWND parent) {
    handle = CreateWindowExA(WS_EX_CONTROLPARENT, PANEL_CLASS, "", WS_CHILD | WS_CLIPSIBLINGS,
                             0, 0, 100, 100, parent, NULL, GetModuleHandleA(NULL), this);
    if (handle == NULL) {
        return false;
    }
    tooltip = CreateWindowExA(WS_EX_TOPMOST, TOOLTIPS_CLASSA, NULL, WS_POPUP | TTS_ALWAYSTIP,
                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                              handle, NULL, GetModuleHandleA(NULL), NULL);
    SendMessageA(tooltip, TTM_SETMAXTIPWIDTH, 0, 320);
    buildControls();
    return true;
}

HWND Panel::getHandle() const {
    return handle;
}

void Panel::setBounds(int x, int y, int width, int height) {
    SetWindowPos(handle, HWND_TOP, x, y, width, height, SWP_NOACTIVATE);
}

void Panel::setVisible(bool visible) {
    ShowWindow(handle, visible ? SW_SHOW : SW_HIDE);
}

void Panel::relayout() {
    if (layout != NULL) {
        RECT area;
        GetClientRect(handle, &area);
        layout->setGeometry(0, 0, area.right, area.bottom);
        InvalidateRect(handle, NULL, TRUE);
    }
}

LRESULT Panel::onNotify(NMHDR*) {
    return 0;
}

COLORREF Panel::labelColour(HWND) {
    return RGB(30, 30, 30);
}

controller::ClinicManager& Panel::manager() {
    return owner->getManager();
}

LRESULT CALLBACK Panel::PanelProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        CREATESTRUCTA* info = (CREATESTRUCTA*)lParam;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)info->lpCreateParams);
    }
    Panel* panel = (Panel*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (panel != NULL) {
        switch (message) {
        case WM_COMMAND:
            panel->onCommand(LOWORD(wParam), HIWORD(wParam));
            return 0;
        case WM_NOTIFY:
            return panel->onNotify((NMHDR*)lParam);
        case WM_SIZE:
            if (panel->layout != NULL) {
                panel->layout->setGeometry(0, 0, LOWORD(lParam), HIWORD(lParam));
            }
            return 0;
        case WM_CTLCOLORSTATIC: {
            HDC dc = (HDC)wParam;
            SetBkColor(dc, RGB(255, 255, 255));
            SetTextColor(dc, panel->labelColour((HWND)lParam));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        }
    }
    return DefWindowProcA(hwnd, message, wParam, lParam);
}

// ---------- making controls ----------

HWND Panel::makeControl(const char* className, const std::string& text, int style, int exStyle, int id) {
    HWND control = CreateWindowExA(exStyle, className, text.c_str(), WS_CHILD | WS_VISIBLE | style,
                                   0, 0, 10, 10, handle, (HMENU)(UINT_PTR)id,
                                   GetModuleHandleA(NULL), NULL);
    SendMessageA(control, WM_SETFONT, (WPARAM)owner->getFont(), TRUE);
    return control;
}

HWND Panel::makeLabel(const std::string& text, bool multiLine) {
    return makeControl("STATIC", text, multiLine ? SS_LEFT : (SS_LEFT | SS_CENTERIMAGE), 0, 0);
}

HWND Panel::makeEdit(int id, int style) {
    return makeControl("EDIT", "", WS_TABSTOP | ES_AUTOHSCROLL | style, WS_EX_CLIENTEDGE, id);
}

HWND Panel::makeButton(int id, const std::string& text, const std::string& tip) {
    HWND button = makeControl("BUTTON", text, WS_TABSTOP | BS_PUSHBUTTON, 0, id);
    if (!tip.empty()) {
        addTip(button, tip);
    }
    return button;
}

HWND Panel::makeCombo(int id, bool editable) {
    int style = WS_TABSTOP | WS_VSCROLL | (editable ? (CBS_DROPDOWN | CBS_AUTOHSCROLL) : CBS_DROPDOWNLIST);
    return makeControl("COMBOBOX", "", style, 0, id);
}

HWND Panel::makeGroup(const std::string& text) {
    HWND group = makeControl("BUTTON", text, BS_GROUPBOX, 0, 0);
    SendMessageA(group, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    return group;
}

HWND Panel::makeListView(int id) {
    HWND list = makeControl(WC_LISTVIEWA, "", WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                            WS_EX_CLIENTEDGE, id);
    SendMessageA(list, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    return list;
}

HWND Panel::makeListBox(int id) {
    return makeControl("LISTBOX", "", WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                       WS_EX_CLIENTEDGE, id);
}

void Panel::addTip(HWND control, const std::string& text) {
    TOOLINFOA info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    info.hwnd = handle;
    info.uId = (UINT_PTR)control;
    info.lpszText = (LPSTR)text.c_str();
    SendMessageA(tooltip, TTM_ADDTOOLA, 0, (LPARAM)&info);
}

// ---------- reading and writing controls ----------

void Panel::addComboItem(HWND combo, const std::string& text) {
    SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM)text.c_str());
}

int Panel::comboIndex(HWND combo) const {
    return (int)SendMessageA(combo, CB_GETCURSEL, 0, 0);
}

std::string Panel::getText(HWND control) const {
    char buffer[512];
    int length = GetWindowTextA(control, buffer, sizeof(buffer));
    return std::string(buffer, length);
}

void Panel::setText(HWND control, const std::string& text) {
    SetWindowTextA(control, text.c_str());
}

// multi line edit boxes need \r\n instead of \n
std::string Panel::windowsText(const std::string& text) const {
    std::string result = "";
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            result += "\r\n";
        } else {
            result += text[i];
        }
    }
    return result;
}

// ---------- list views ----------

void Panel::addColumn(HWND list, int index, const std::string& title, int width) {
    LVCOLUMNA column;
    ZeroMemory(&column, sizeof(column));
    column.mask = LVCF_TEXT | LVCF_WIDTH;
    column.pszText = (LPSTR)title.c_str();
    column.cx = width;
    SendMessageA(list, LVM_INSERTCOLUMNA, index, (LPARAM)&column);
}

void Panel::addRow(HWND list, const std::vector<std::string>& cells, int colour) {
    int row = (int)SendMessageA(list, LVM_GETITEMCOUNT, 0, 0);
    LVITEMA item;
    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = row;
    item.pszText = (LPSTR)cells[0].c_str();
    item.lParam = colour;
    SendMessageA(list, LVM_INSERTITEMA, 0, (LPARAM)&item);
    for (size_t i = 1; i < cells.size(); i++) {
        LVITEMA cell;
        ZeroMemory(&cell, sizeof(cell));
        cell.iSubItem = (int)i;
        cell.pszText = (LPSTR)cells[i].c_str();
        SendMessageA(list, LVM_SETITEMTEXTA, row, (LPARAM)&cell);
    }
}

void Panel::clearList(HWND list) {
    SendMessageA(list, LVM_DELETEALLITEMS, 0, 0);
}

int Panel::selectedRow(HWND list) const {
    return (int)SendMessageA(list, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
}

std::string Panel::cellText(HWND list, int row, int column) const {
    char buffer[256];
    buffer[0] = '\0';
    LVITEMA item;
    ZeroMemory(&item, sizeof(item));
    item.iSubItem = column;
    item.pszText = buffer;
    item.cchTextMax = sizeof(buffer);
    SendMessageA(list, LVM_GETITEMTEXTA, row, (LPARAM)&item);
    return buffer;
}

// custom drawing so each row gets the colour stored in its lParam
LRESULT Panel::paintRow(NMHDR* header) {
    NMLVCUSTOMDRAW* draw = (NMLVCUSTOMDRAW*)header;
    if (draw->nmcd.dwDrawStage == CDDS_PREPAINT) {
        return CDRF_NOTIFYITEMDRAW;
    }
    if (draw->nmcd.dwDrawStage != CDDS_ITEMPREPAINT) {
        return CDRF_DODEFAULT;
    }
    switch ((int)draw->nmcd.lItemlParam) {
    case ROW_RED:
        draw->clrTextBk = RGB(255, 222, 222);
        draw->clrText = RGB(150, 0, 0);
        break;
    case ROW_BLUE:
        draw->clrTextBk = RGB(222, 235, 255);
        draw->clrText = RGB(0, 60, 140);
        break;
    case ROW_YELLOW:
        draw->clrTextBk = RGB(255, 247, 209);
        draw->clrText = RGB(100, 75, 0);
        break;
    case ROW_GREEN:
        draw->clrTextBk = RGB(223, 245, 223);
        draw->clrText = RGB(0, 100, 0);
        break;
    case ROW_GREY:
        draw->clrTextBk = RGB(236, 236, 236);
        draw->clrText = RGB(125, 125, 125);
        break;
    default:
        return CDRF_DODEFAULT;
    }
    return CDRF_NEWFONT;
}

int Panel::colourFor(const std::string& categoryOrStatus) const {
    if (categoryOrStatus == "Emergency") {
        return ROW_RED;
    }
    if (categoryOrStatus == "Follow-Up") {
        return ROW_BLUE;
    }
    if (categoryOrStatus == "Scheduled") {
        return ROW_YELLOW;
    }
    if (categoryOrStatus == "Completed") {
        return ROW_GREEN;
    }
    if (categoryOrStatus == "Cancelled") {
        return ROW_GREY;
    }
    return ROW_NORMAL;
}

// ---------- dialogs ----------

void Panel::showInfo(const std::string& title, const std::string& message) {
    MessageBoxA(owner->getHandle(), message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}

void Panel::showError(const std::string& title, const std::string& message) {
    MessageBoxA(owner->getHandle(), message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
}

void Panel::showException(const std::exception& error) {
    const model::ClinicException* clinicError = dynamic_cast<const model::ClinicException*>(&error);
    std::string title = clinicError != NULL ? clinicError->getTitle() : "Error";
    showError(title, std::string("Error: ") + error.what());
}

bool Panel::askYesNo(const std::string& title, const std::string& question) {
    return MessageBoxA(owner->getHandle(), question.c_str(), title.c_str(),
                       MB_YESNO | MB_ICONQUESTION) == IDYES;
}

}
