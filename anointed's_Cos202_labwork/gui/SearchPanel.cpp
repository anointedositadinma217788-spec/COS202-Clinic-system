#include "SearchPanel.h"
#include "MainWindow.h"
#include "../utils/Validator.h"

namespace gui {

enum SearchId {
    IDC_SRC_FIELD = 100,
    IDC_SRC_METHOD,
    IDC_SRC_KEYWORD,
    IDC_SRC_SEARCH,
    IDC_SRC_SHOWALL,
    IDC_SRT_ALGORITHM,
    IDC_SRT_FIELD,
    IDC_SRT_SORT,
    IDC_SRC_RESULTS,
    IDC_SRC_RECENT
};

SearchPanel::SearchPanel(MainWindow* owner) : Panel(owner) {}

void SearchPanel::buildControls() {
    BoxLayout* root = new BoxLayout(true, 8, 12);
    BoxLayout* top = new BoxLayout(false, 12);

    HWND searchBox = makeGroup("Search Patients");
    BoxLayout* searchInside = new BoxLayout(true, 6);
    GridLayout* searchForm = new GridLayout(2, 24, 6);
    searchForm->setColumnWidth(0, 110);

    fieldCombo = makeCombo(IDC_SRC_FIELD, false);
    addComboItem(fieldCombo, "Patient ID");
    addComboItem(fieldCombo, "Patient Name");
    addComboItem(fieldCombo, "Category");
    SendMessageA(fieldCombo, CB_SETCURSEL, 1, 0);
    searchForm->addWidget(makeLabel("Search By:"), 0, 0);
    searchForm->addWidget(fieldCombo, 0, 1);

    methodCombo = makeCombo(IDC_SRC_METHOD, false);
    addComboItem(methodCombo, "Auto (best for current data)");
    addComboItem(methodCombo, "Linear Search");
    addComboItem(methodCombo, "Binary Search");
    addComboItem(methodCombo, "Recursive Search");
    SendMessageA(methodCombo, CB_SETCURSEL, 0, 0);
    addTip(methodCombo, "Binary Search needs the records sorted by Patient ID first");
    searchForm->addWidget(makeLabel("Algorithm:"), 1, 0);
    searchForm->addWidget(methodCombo, 1, 1);

    keywordEdit = makeEdit(IDC_SRC_KEYWORD);
    addTip(keywordEdit, "Ctrl+F jumps here. Category can be General, Emergency or Follow-Up");
    BoxLayout* keywordRow = new BoxLayout(false, 6);
    keywordRow->addWidget(keywordEdit, 100, 1);
    keywordRow->addWidget(makeButton(IDC_SRC_SEARCH, "Search", "Run the search (Ctrl+F to jump to the keyword box)"), 80);
    keywordRow->addWidget(makeButton(IDC_SRC_SHOWALL, "Show All", "List every patient in the current order"), 80);
    searchForm->addWidget(makeLabel("Keyword:"), 2, 0);
    searchForm->addLayout(keywordRow, 2, 1);
    searchInside->addLayout(searchForm, 90);

    searchHint = makeLabel("", true);
    searchInside->addWidget(searchHint, 34, 1);
    top->addLayout(new GroupLayout(searchBox, searchInside), 420, 1);

    HWND sortBox = makeGroup("Sort Patients");
    BoxLayout* sortInside = new BoxLayout(true, 6);
    GridLayout* sortForm = new GridLayout(2, 24, 6);
    sortForm->setColumnWidth(0, 110);

    sortAlgorithmCombo = makeCombo(IDC_SRT_ALGORITHM, false);
    addComboItem(sortAlgorithmCombo, "Selection Sort");
    addComboItem(sortAlgorithmCombo, "Insertion Sort");
    addComboItem(sortAlgorithmCombo, "Merge Sort");
    SendMessageA(sortAlgorithmCombo, CB_SETCURSEL, 0, 0);
    sortForm->addWidget(makeLabel("Sort Algorithm:"), 0, 0);
    sortForm->addWidget(sortAlgorithmCombo, 0, 1);

    sortFieldCombo = makeCombo(IDC_SRT_FIELD, false);
    addComboItem(sortFieldCombo, "Patient Name");
    addComboItem(sortFieldCombo, "Patient ID");
    addComboItem(sortFieldCombo, "Age");
    addComboItem(sortFieldCombo, "Appointment Date");
    SendMessageA(sortFieldCombo, CB_SETCURSEL, 0, 0);
    sortForm->addWidget(makeLabel("Sort By:"), 1, 0);
    sortForm->addWidget(sortFieldCombo, 1, 1);

    BoxLayout* sortRow = new BoxLayout(false, 6);
    sortRow->addWidget(makeButton(IDC_SRT_SORT, "SORT", "Sort all patient records with the chosen algorithm"), 120);
    sortRow->addSpace(10, 1);
    sortForm->addLayout(sortRow, 2, 1);
    sortInside->addLayout(sortForm, 90);

    explainLabel = makeLabel("", true);
    sortInside->addWidget(explainLabel, 34, 1);
    top->addLayout(new GroupLayout(sortBox, sortInside), 420, 1);

    root->addLayout(top, 180);

    sortState = makeLabel("");
    SendMessageA(sortState, WM_SETFONT, (WPARAM)owner->getBoldFont(), TRUE);
    root->addWidget(sortState, 20);

    BoxLayout* bottom = new BoxLayout(false, 12);
    BoxLayout* results = new BoxLayout(true, 4);
    resultTitle = makeLabel("Results");
    results->addWidget(resultTitle, 20);
    resultList = makeListView(IDC_SRC_RESULTS);
    addColumn(resultList, 0, "#", 35);
    addColumn(resultList, 1, "Patient ID", 75);
    addColumn(resultList, 2, "Name", 165);
    addColumn(resultList, 3, "Age", 45);
    addColumn(resultList, 4, "Category", 85);
    addColumn(resultList, 5, "Contact", 110);
    addColumn(resultList, 6, "Next Appointment", 125);
    results->addWidget(resultList, 100, 1);
    bottom->addLayout(results, 400, 1);

    BoxLayout* recent = new BoxLayout(true, 4);
    recent->addWidget(makeLabel("Recently Accessed (array cache of 10)"), 20);
    recentList = makeListBox(IDC_SRC_RECENT);
    addTip(recentList, "Patients you opened or searched for last. An Auto search by ID checks here first.");
    recent->addWidget(recentList, 100, 1);
    bottom->addLayout(recent, 250);
    root->addLayout(bottom, 100, 1);

    layout = root;
    updateHint();
}

void SearchPanel::refresh() {
    setText(sortState, manager().getSortState());
    refreshRecent();
    updateHint();
}

void SearchPanel::focusKeyword() {
    SetFocus(keywordEdit);
    SendMessageA(keywordEdit, EM_SETSEL, 0, -1);
}

void SearchPanel::refreshRecent() {
    const model::ClinicDatabase& database = manager().getDatabase();
    SendMessageA(recentList, LB_RESETCONTENT, 0, 0);
    for (int i = 0; i < database.getRecentCount(); i++) {
        model::PatientRecord* patient = database.getRecent(i);
        std::string line = utils::intToStr(i + 1) + ".  " + patient->getId() + "  " +
                           patient->getName() + "  (" + patient->getCategory() + ")";
        SendMessageA(recentList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }
    if (database.getRecentCount() == 0) {
        SendMessageA(recentList, LB_ADDSTRING, 0, (LPARAM)"(empty - open or search for a patient)");
    }
}

// text field and combo box events update this hint straight away
void SearchPanel::updateHint() {
    std::string hint = manager().describeSearch(comboIndex(fieldCombo), comboIndex(methodCombo));
    std::string keyword = utils::trim(getText(keywordEdit));
    if (!keyword.empty()) {
        hint = "Searching for \"" + keyword + "\". " + hint;
    }
    setText(searchHint, hint);
    setText(explainLabel, manager().explainSort(comboIndex(sortAlgorithmCombo)));
}

void SearchPanel::showResults(const std::vector<model::PatientRecord*>& patients) {
    clearList(resultList);
    for (size_t i = 0; i < patients.size(); i++) {
        model::PatientRecord* p = patients[i];
        std::string next = p->getNextAppointment();
        std::vector<std::string> cells;
        cells.push_back(utils::intToStr((int)i + 1));
        cells.push_back(p->getId());
        cells.push_back(p->getName());
        cells.push_back(utils::intToStr(p->getAge()));
        cells.push_back(p->getCategory());
        cells.push_back(p->getContact());
        cells.push_back(next.empty() ? "-" : next);
        addRow(resultList, cells, colourFor(p->getCategory()));
    }
}

void SearchPanel::doSearch() {
    try {
        std::string methodUsed;
        std::vector<model::PatientRecord*> found = manager().searchPatients(
            comboIndex(fieldCombo), comboIndex(methodCombo), getText(keywordEdit), methodUsed);
        showResults(found);
        setText(resultTitle, "Results: " + utils::intToStr((int)found.size()) + " match(es) using " + methodUsed);
        refreshRecent();
        owner->setStatus("Search finished with " + methodUsed + ".");
        if (found.empty()) {
            showInfo("Search", "No patient matched \"" + utils::trim(getText(keywordEdit)) + "\".");
        }
    } catch (const std::exception& error) {
        showException(error);
    } catch (...) {
        showError("Error", "Error: Something unexpected went wrong while searching.");
    }
}

void SearchPanel::doSort() {
    try {
        int algorithm = comboIndex(sortAlgorithmCombo);
        int field = comboIndex(sortFieldCombo);
        manager().sortPatients(algorithm, field);
        showResults(manager().getPatients());
        std::string text = manager().getSortState();
        setText(resultTitle, "All patients after sorting - " + text);
        owner->refreshAll();
        owner->setStatus(text);
    } catch (const std::exception& error) {
        showException(error);
    }
}

void SearchPanel::onCommand(int id, int code) {
    if (id == IDC_SRC_KEYWORD && code == EN_CHANGE) {
        updateHint();
        return;
    }
    if ((id == IDC_SRC_FIELD || id == IDC_SRC_METHOD || id == IDC_SRT_ALGORITHM) && code == CBN_SELCHANGE) {
        updateHint();
        return;
    }
    if (code != BN_CLICKED) {
        return;
    }
    if (id == IDC_SRC_SEARCH) {
        doSearch();
    } else if (id == IDC_SRT_SORT) {
        doSort();
    } else if (id == IDC_SRC_SHOWALL) {
        showResults(manager().getPatients());
        setText(resultTitle, "All patients (" + utils::intToStr((int)manager().getPatients().size()) + ")");
    }
}

LRESULT SearchPanel::onNotify(NMHDR* header) {
    if (header->hwndFrom == resultList && header->code == NM_CUSTOMDRAW) {
        return paintRow(header);
    }
    return 0;
}

COLORREF SearchPanel::labelColour(HWND control) {
    if (control == searchHint || control == explainLabel) {
        return RGB(70, 70, 120);
    }
    if (control == sortState) {
        return RGB(20, 110, 80);
    }
    return Panel::labelColour(control);
}

}
