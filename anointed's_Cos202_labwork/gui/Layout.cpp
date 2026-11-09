#include "Layout.h"

namespace gui {

static bool isShown(HWND window) {
    return (GetWindowLongA(window, GWL_STYLE) & WS_VISIBLE) != 0;
}

static void placeWindow(HWND window, int x, int y, int width, int height) {
    char className[32];
    GetClassNameA(window, className, sizeof(className));
    // the height of a combo box also sets how long its drop down list is
    if (lstrcmpiA(className, "ComboBox") == 0) {
        height = 260;
    }
    MoveWindow(window, x, y, width, height, TRUE);
}

static void placeItem(const LayoutItem& item, int x, int y, int width, int height) {
    if (width < 0) {
        width = 0;
    }
    if (height < 0) {
        height = 0;
    }
    if (item.window != NULL) {
        placeWindow(item.window, x, y, width, height);
    } else if (item.child != NULL) {
        item.child->setGeometry(x, y, width, height);
    }
}

// ---------- BoxLayout ----------

BoxLayout::BoxLayout(bool vertical, int spacing, int margin)
    : vertical(vertical), spacing(spacing), margin(margin) {}

BoxLayout::~BoxLayout() {
    for (size_t i = 0; i < items.size(); i++) {
        delete items[i].child;
    }
}

void BoxLayout::addWidget(HWND window, int size, int stretch) {
    LayoutItem item;
    item.window = window;
    item.child = NULL;
    item.size = size;
    item.stretch = stretch;
    items.push_back(item);
}

void BoxLayout::addLayout(Layout* layout, int size, int stretch) {
    LayoutItem item;
    item.window = NULL;
    item.child = layout;
    item.size = size;
    item.stretch = stretch;
    items.push_back(item);
}

void BoxLayout::addSpace(int size, int stretch) {
    LayoutItem item;
    item.window = NULL;
    item.child = NULL;
    item.size = size;
    item.stretch = stretch;
    items.push_back(item);
}

void BoxLayout::setGeometry(int x, int y, int width, int height) {
    x += margin;
    y += margin;
    width -= margin * 2;
    height -= margin * 2;

    // first add up the fixed sizes, whatever is left goes to stretch items
    int fixedTotal = 0;
    int stretchTotal = 0;
    int count = 0;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].window != NULL && !isShown(items[i].window)) {
            continue;
        }
        fixedTotal += items[i].size;
        stretchTotal += items[i].stretch;
        count++;
    }
    if (count > 1) {
        fixedTotal += spacing * (count - 1);
    }
    int extra = (vertical ? height : width) - fixedTotal;
    if (extra < 0) {
        extra = 0;
    }

    int position = vertical ? y : x;
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].window != NULL && !isShown(items[i].window)) {
            continue;
        }
        int size = items[i].size;
        if (stretchTotal > 0 && items[i].stretch > 0) {
            size += extra * items[i].stretch / stretchTotal;
        }
        if (vertical) {
            placeItem(items[i], x, position, width, size);
        } else {
            placeItem(items[i], position, y, size, height);
        }
        position += size + spacing;
    }
}

// ---------- GridLayout ----------

GridLayout::GridLayout(int columns, int rowHeight, int spacing)
    : columns(columns), rowHeight(rowHeight), spacing(spacing) {
    for (int i = 0; i < columns; i++) {
        columnWidths.push_back(0);
    }
}

GridLayout::~GridLayout() {
    for (size_t i = 0; i < cells.size(); i++) {
        delete cells[i].item.child;
    }
}

// width 0 means the column stretches
void GridLayout::setColumnWidth(int column, int width) {
    if (column >= 0 && column < columns) {
        columnWidths[column] = width;
    }
}

void GridLayout::addWidget(HWND window, int row, int column, int span) {
    GridCell cell;
    cell.item.window = window;
    cell.item.child = NULL;
    cell.item.size = 0;
    cell.item.stretch = 0;
    cell.row = row;
    cell.column = column;
    cell.span = span;
    cells.push_back(cell);
}

void GridLayout::addLayout(Layout* layout, int row, int column, int span) {
    GridCell cell;
    cell.item.window = NULL;
    cell.item.child = layout;
    cell.item.size = 0;
    cell.item.stretch = 0;
    cell.row = row;
    cell.column = column;
    cell.span = span;
    cells.push_back(cell);
}

void GridLayout::setGeometry(int x, int y, int width, int height) {
    int fixedTotal = spacing * (columns - 1);
    int stretchColumns = 0;
    for (int c = 0; c < columns; c++) {
        if (columnWidths[c] > 0) {
            fixedTotal += columnWidths[c];
        } else {
            stretchColumns++;
        }
    }
    int stretchWidth = 0;
    if (stretchColumns > 0 && width > fixedTotal) {
        stretchWidth = (width - fixedTotal) / stretchColumns;
    }

    std::vector<int> columnX;
    std::vector<int> columnW;
    int currentX = x;
    for (int c = 0; c < columns; c++) {
        int w = columnWidths[c] > 0 ? columnWidths[c] : stretchWidth;
        columnX.push_back(currentX);
        columnW.push_back(w);
        currentX += w + spacing;
    }

    int lastRow = 0;
    for (size_t i = 0; i < cells.size(); i++) {
        if (cells[i].row > lastRow) {
            lastRow = cells[i].row;
        }
    }

    int currentY = y;
    for (int r = 0; r <= lastRow; r++) {
        bool rowShown = false;
        for (size_t i = 0; i < cells.size(); i++) {
            if (cells[i].row == r && (cells[i].item.child != NULL || isShown(cells[i].item.window))) {
                rowShown = true;
            }
        }
        if (!rowShown) {
            continue;
        }
        for (size_t i = 0; i < cells.size(); i++) {
            if (cells[i].row != r) {
                continue;
            }
            int column = cells[i].column;
            int w = 0;
            for (int k = column; k < column + cells[i].span && k < columns; k++) {
                w += columnW[k];
                if (k > column) {
                    w += spacing;
                }
            }
            placeItem(cells[i].item, columnX[column], currentY, w, rowHeight);
        }
        currentY += rowHeight + spacing;
    }
    (void)height;
}

// ---------- GroupLayout ----------

GroupLayout::GroupLayout(HWND box, Layout* inner) : box(box), inner(inner) {}

GroupLayout::~GroupLayout() {
    delete inner;
}

void GroupLayout::setGeometry(int x, int y, int width, int height) {
    MoveWindow(box, x, y, width, height, TRUE);
    if (inner != NULL) {
        inner->setGeometry(x + 12, y + 22, width - 24, height - 32);
    }
}

}
