#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "../model/PatientRecord.h"
#include <string>
#include <vector>

namespace controller {

// these numbers match the order of the items in the GUI combo boxes
enum SearchField {
    FIELD_ID = 0,
    FIELD_NAME = 1,
    FIELD_CATEGORY = 2
};

enum SearchMethod {
    SEARCH_AUTO = 0,
    SEARCH_LINEAR = 1,
    SEARCH_BINARY = 2,
    SEARCH_RECURSIVE = 3
};

class SearchEngine {
public:
    std::vector<model::PatientRecord*> linearSearch(const std::vector<model::PatientRecord*>& list,
                                                    int field, const std::string& keyword) const;
    model::PatientRecord* binarySearchById(const std::vector<model::PatientRecord*>& list,
                                           const std::string& patientId) const;
    std::vector<model::PatientRecord*> recursiveSearch(const std::vector<model::PatientRecord*>& list,
                                                       int field, const std::string& keyword) const;

    bool isCategoryName(const std::string& text) const;

private:
    bool matches(const model::PatientRecord* patient, int field, const std::string& keyword) const;
    std::string simpleCategory(const std::string& text) const;
    void recursiveHelper(const std::vector<model::PatientRecord*>& list, size_t index, int field,
                         const std::string& keyword,
                         std::vector<model::PatientRecord*>& found) const;
};

}

#endif
