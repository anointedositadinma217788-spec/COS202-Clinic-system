#include "SortEngine.h"
#include "../utils/Validator.h"

namespace controller {

void SortEngine::sort(std::vector<model::PatientRecord*>& list, int algorithm, int field) const {
    if (algorithm == SELECTION_SORT) {
        selectionSort(list, field);
    } else if (algorithm == INSERTION_SORT) {
        insertionSort(list, field);
    } else {
        mergeSort(list, 0, (int)list.size() - 1, field);
    }
}

// true when a should be placed before b
bool SortEngine::comesBefore(const model::PatientRecord* a, const model::PatientRecord* b,
                             int field) const {
    if (field == SORT_NAME) {
        return utils::toLower(a->getName()) < utils::toLower(b->getName());
    }
    if (field == SORT_ID) {
        return a->getId() < b->getId();
    }
    if (field == SORT_AGE) {
        return a->getAge() < b->getAge();
    }
    // patients without a booked appointment go to the end
    std::string first = a->getNextAppointment();
    std::string second = b->getNextAppointment();
    if (first.empty()) {
        return false;
    }
    if (second.empty()) {
        return true;
    }
    return first < second;
}

// selection sort: find the smallest item and swap it to the front
void SortEngine::selectionSort(std::vector<model::PatientRecord*>& list, int field) const {
    int size = (int)list.size();
    for (int i = 0; i < size - 1; i++) {
        int smallest = i;
        for (int j = i + 1; j < size; j++) {
            if (comesBefore(list[j], list[smallest], field)) {
                smallest = j;
            }
        }
        if (smallest != i) {
            model::PatientRecord* temp = list[i];
            list[i] = list[smallest];
            list[smallest] = temp;
        }
    }
}

// insertion sort: take each item and slide it back into its place
void SortEngine::insertionSort(std::vector<model::PatientRecord*>& list, int field) const {
    for (int i = 1; i < (int)list.size(); i++) {
        model::PatientRecord* current = list[i];
        int j = i - 1;
        while (j >= 0 && comesBefore(current, list[j], field)) {
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = current;
    }
}

// merge sort: split in half, sort both halves (recursively), then merge
void SortEngine::mergeSort(std::vector<model::PatientRecord*>& list, int left, int right,
                           int field) const {
    if (left >= right) {
        return;
    }
    int middle = left + (right - left) / 2;
    mergeSort(list, left, middle, field);
    mergeSort(list, middle + 1, right, field);
    merge(list, left, middle, right, field);
}

void SortEngine::merge(std::vector<model::PatientRecord*>& list, int left, int middle, int right,
                       int field) const {
    std::vector<model::PatientRecord*> leftPart(list.begin() + left, list.begin() + middle + 1);
    std::vector<model::PatientRecord*> rightPart(list.begin() + middle + 1, list.begin() + right + 1);

    size_t i = 0;
    size_t j = 0;
    int k = left;
    while (i < leftPart.size() && j < rightPart.size()) {
        if (comesBefore(rightPart[j], leftPart[i], field)) {
            list[k] = rightPart[j];
            j++;
        } else {
            list[k] = leftPart[i];
            i++;
        }
        k++;
    }
    while (i < leftPart.size()) {
        list[k] = leftPart[i];
        i++;
        k++;
    }
    while (j < rightPart.size()) {
        list[k] = rightPart[j];
        j++;
        k++;
    }
}

std::string SortEngine::algorithmName(int algorithm) const {
    if (algorithm == SELECTION_SORT) {
        return "Selection Sort";
    }
    if (algorithm == INSERTION_SORT) {
        return "Insertion Sort";
    }
    return "Merge Sort";
}

std::string SortEngine::fieldName(int field) const {
    if (field == SORT_NAME) {
        return "Patient Name";
    }
    if (field == SORT_ID) {
        return "Patient ID";
    }
    if (field == SORT_AGE) {
        return "Age";
    }
    return "Appointment Date";
}

std::string SortEngine::explain(int algorithm) const {
    if (algorithm == SELECTION_SORT) {
        return "Selection Sort - O(n^2). Makes the fewest swaps (at most n-1), which is good for "
               "the small patient list of a university clinic. Simple and easy to follow.";
    }
    if (algorithm == INSERTION_SORT) {
        return "Insertion Sort - O(n^2), but almost O(n) when the list is already nearly sorted. "
               "Best after adding a few new patients to a list that was sorted before. Stable.";
    }
    return "Merge Sort - O(n log n) every time and stable (equal records keep their order). "
           "Best choice when the clinic has many patients. Uses recursion and extra memory.";
}

}
