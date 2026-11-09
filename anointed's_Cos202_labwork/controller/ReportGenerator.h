#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "../model/ClinicDatabase.h"
#include <string>

namespace controller {

class ReportGenerator {
private:
    const model::ClinicDatabase& database;

    int countCategoryFrom(size_t index, const std::string& category) const;
    std::string header(const std::string& title) const;

public:
    ReportGenerator(const model::ClinicDatabase& database);

    std::string mostAttended() const;
    std::string pendingAppointments() const;
    std::string categoryDistribution() const;
    std::string dailySummary(const std::string& date) const;
    std::string feeSummary() const;

    int countCategory(const std::string& category) const;
};

}

#endif
