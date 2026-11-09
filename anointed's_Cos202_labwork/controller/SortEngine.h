#ifndef SORTENGINE_H
#define SORTENGINE_H

#include "../model/PatientRecord.h"
#include <string>
#include <vector>

namespace controller {

// these numbers match the order of the items in the GUI combo boxes
enum SortAlgorithm {
    SELECTION_SORT = 0,
    INSERTION_SORT = 1,
    MERGE_SORT = 2
};

enum SortField {
    SORT_NAME = 0,
    SORT_ID = 1,
    SORT_AGE = 2,
    SORT_APPOINTMENT = 3
};

class SortEngine {
public:
    void sort(std::vector<model::PatientRecord*>& list, int algorithm, int field) const;

    std::string algorithmName(int algorithm) const;
    std::string fieldName(int field) const;
    std::string explain(int algorithm) const;

private:
    bool comesBefore(const model::PatientRecord* a, const model::PatientRecord* b, int field) const;

    void selectionSort(std::vector<model::PatientRecord*>& list, int field) const;
    void insertionSort(std::vector<model::PatientRecord*>& list, int field) const;
    void mergeSort(std::vector<model::PatientRecord*>& list, int left, int right, int field) const;
    void merge(std::vector<model::PatientRecord*>& list, int left, int middle, int right,
               int field) const;
};

}

#endif
