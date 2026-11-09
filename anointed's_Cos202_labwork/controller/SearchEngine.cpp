#include "SearchEngine.h"
#include "../utils/Validator.h"

namespace controller {

// "Follow-Up", "follow up" and "followup" all become "followup"
std::string SearchEngine::simpleCategory(const std::string& text) const {
    std::string lowered = utils::toLower(utils::trim(text));
    std::string result = "";
    for (size_t i = 0; i < lowered.size(); i++) {
        if (lowered[i] != '-' && lowered[i] != ' ') {
            result += lowered[i];
        }
    }
    return result;
}

bool SearchEngine::isCategoryName(const std::string& text) const {
    std::string category = simpleCategory(text);
    return category == "general" || category == "emergency" || category == "followup";
}

// ID and category must match fully, a name can match only part
bool SearchEngine::matches(const model::PatientRecord* patient, int field,
                           const std::string& keyword) const {
    if (field == FIELD_ID) {
        return utils::toUpper(patient->getId()) == utils::toUpper(keyword);
    }
    if (field == FIELD_NAME) {
        return utils::toLower(patient->getName()).find(utils::toLower(keyword)) != std::string::npos;
    }
    return simpleCategory(patient->getCategory()) == simpleCategory(keyword);
}

// linear search: check every record one by one, works on unsorted data
std::vector<model::PatientRecord*> SearchEngine::linearSearch(
        const std::vector<model::PatientRecord*>& list, int field, const std::string& keyword) const {
    std::vector<model::PatientRecord*> found;
    for (size_t i = 0; i < list.size(); i++) {
        if (matches(list[i], field, keyword)) {
            found.push_back(list[i]);
        }
    }
    return found;
}

// binary search: list must already be sorted by patient ID
model::PatientRecord* SearchEngine::binarySearchById(const std::vector<model::PatientRecord*>& list,
                                                     const std::string& patientId) const {
    int low = 0;
    int high = (int)list.size() - 1;
    while (low <= high) {
        int middle = low + (high - low) / 2;
        std::string middleId = list[middle]->getId();
        if (middleId == patientId) {
            return list[middle];
        }
        if (middleId < patientId) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return NULL;
}

// recursive search: checks one record then calls itself for the rest
std::vector<model::PatientRecord*> SearchEngine::recursiveSearch(
        const std::vector<model::PatientRecord*>& list, int field, const std::string& keyword) const {
    std::vector<model::PatientRecord*> found;
    recursiveHelper(list, 0, field, keyword, found);
    return found;
}

void SearchEngine::recursiveHelper(const std::vector<model::PatientRecord*>& list, size_t index,
                                   int field, const std::string& keyword,
                                   std::vector<model::PatientRecord*>& found) const {
    if (index >= list.size()) {
        return;
    }
    if (matches(list[index], field, keyword)) {
        found.push_back(list[index]);
    }
    recursiveHelper(list, index + 1, field, keyword, found);
}

}
