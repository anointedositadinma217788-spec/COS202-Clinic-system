#ifndef LAYOUT_H
#define LAYOUT_H

#include "WinHeaders.h"
#include <vector>

namespace gui {

// Win32 has no layout managers so these classes position the controls
// every time a page is resized
class Layout {
public:
    virtual ~Layout() {}
    virtual void setGeometry(int x, int y, int width, int height) = 0;
};

struct LayoutItem {
    HWND window;
    Layout* child;
    int size;
    int stretch;
};

// puts items in a row (horizontal) or a column (vertical)
class BoxLayout : public Layout {
private:
    bool vertical;
    int spacing;
    int margin;
    std::vector<LayoutItem> items;

public:
    BoxLayout(bool vertical, int spacing = 6, int margin = 0);
    ~BoxLayout();

    void addWidget(HWND window, int size, int stretch = 0);
    void addLayout(Layout* layout, int size, int stretch = 0);
    void addSpace(int size, int stretch = 0);
    void setGeometry(int x, int y, int width, int height);
};

struct GridCell {
    LayoutItem item;
    int row;
    int column;
    int span;
};

// rows and columns, mostly used for label + input forms
// rows where every control is hidden take no space
class GridLayout : public Layout {
private:
    int columns;
    int rowHeight;
    int spacing;
    std::vector<int> columnWidths;
    std::vector<GridCell> cells;

public:
    GridLayout(int columns, int rowHeight, int spacing = 6);
    ~GridLayout();

    void setColumnWidth(int column, int width);
    void addWidget(HWND window, int row, int column, int span = 1);
    void addLayout(Layout* layout, int row, int column, int span = 1);
    void setGeometry(int x, int y, int width, int height);
};

// a group box with another layout inside it
class GroupLayout : public Layout {
private:
    HWND box;
    Layout* inner;

public:
    GroupLayout(HWND box, Layout* inner);
    ~GroupLayout();
    void setGeometry(int x, int y, int width, int height);
};

}

#endif
