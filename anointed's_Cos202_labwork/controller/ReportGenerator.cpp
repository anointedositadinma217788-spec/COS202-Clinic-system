#include "ReportGenerator.h"
#include "../model/ClinicExceptions.h"
#include "../utils/Validator.h"
#include <sstream>
#include <iomanip>
#include <vector>

namespace controller {

struct AttendanceRow {
    model::PatientRecord* patient;
    int completed;
};

static std::string shorten(const std::string& text, size_t size) {
    if (text.size() <= size) {
        return text;
    }
    return text.substr(0, size - 3) + "...";
}

ReportGenerator::ReportGenerator(const model::ClinicDatabase& database) : database(database) {}

std::string ReportGenerator::header(const std::string& title) const {
    std::stringstream out;
    out << title << "\n";
    out << "Generated on " << utils::todayDate() << " at " << utils::currentTime() << "\n";
    out << std::string(70, '-') << "\n";
    return out.str();
}

// recursion: count this patient, then call again for the rest of the list
int ReportGenerator::countCategoryFrom(size_t index, const std::string& category) const {
    const std::vector<model::PatientRecord*>& patients = database.getPatients();
    if (index >= patients.size()) {
        return 0;
    }
    int thisOne = patients[index]->getCategory() == category ? 1 : 0;
    return thisOne + countCategoryFrom(index + 1, category);
}

int ReportGenerator::countCategory(const std::string& category) const {
    return countCategoryFrom(0, category);
}

std::string ReportGenerator::mostAttended() const {
    std::stringstream out;
    out << header("REPORT 1: MOST FREQUENTLY ATTENDED PATIENTS");

    const std::vector<model::PatientRecord*>& patients = database.getPatients();
    std::vector<AttendanceRow> rows;
    for (size_t i = 0; i < patients.size(); i++) {
        AttendanceRow row;
        row.patient = patients[i];
        row.completed = patients[i]->countAppointments("Completed");
        if (row.completed > 0) {
            rows.push_back(row);
        }
    }

    // insertion sort, highest number of visits first
    for (int i = 1; i < (int)rows.size(); i++) {
        AttendanceRow current = rows[i];
        int j = i - 1;
        while (j >= 0 && rows[j].completed < current.completed) {
            rows[j + 1] = rows[j];
            j--;
        }
        rows[j + 1] = current;
    }

    if (rows.empty()) {
        out << "No patient has a completed appointment yet.\n";
        return out.str();
    }
    out << std::left << std::setw(6) << "Rank" << std::setw(8) << "ID" << std::setw(26) << "Name"
        << std::setw(12) << "Category" << "Completed Visits\n";
    for (size_t i = 0; i < rows.size() && i < 10; i++) {
        out << std::left << std::setw(6) << (i + 1) << std::setw(8) << rows[i].patient->getId()
            << std::setw(26) << shorten(rows[i].patient->getName(), 24)
            << std::setw(12) << rows[i].patient->getCategory() << rows[i].completed << "\n";
    }
    return out.str();
}

std::string ReportGenerator::pendingAppointments() const {
    std::stringstream out;
    out << header("REPORT 2: PATIENTS WITH PENDING APPOINTMENTS");

    const std::vector<model::PatientRecord*>& patients = database.getPatients();
    int patientCount = 0;
    int apptCount = 0;
    for (size_t i = 0; i < patients.size(); i++) {
        int pending = patients[i]->countAppointments("Scheduled");
        if (pending == 0) {
            continue;
        }
        patientCount++;
        out << patients[i]->getId() << "  " << patients[i]->getName() << " ("
            << patients[i]->getCategory() << ") - " << pending << " pending\n";
        std::vector<model::Appointment*> appts = patients[i]->getAppointments();
        for (size_t j = 0; j < appts.size(); j++) {
            if (appts[j]->isPending()) {
                apptCount++;
                out << "      #" << std::left << std::setw(5) << appts[j]->getNumber()
                    << appts[j]->getDate() << "  " << appts[j]->getTime() << "   "
                    << std::setw(14) << appts[j]->getDoctor() << appts[j]->getService() << "\n";
            }
        }
    }
    if (patientCount == 0) {
        out << "There are no pending appointments.\n";
    } else {
        out << std::string(70, '-') << "\n";
        out << patientCount << " patient(s) with " << apptCount << " pending appointment(s).\n";
    }
    return out.str();
}

std::string ReportGenerator::categoryDistribution() const {
    std::stringstream out;
    out << header("REPORT 3: PATIENT CATEGORY DISTRIBUTION");

    std::string names[3] = {"General", "Emergency", "Follow-Up"};
    int total = (int)database.getPatients().size();
    out << std::left << std::setw(22) << "Category" << std::setw(10) << "Patients"
        << std::setw(10) << "Percent" << "Chart\n";
    for (int i = 0; i < 3; i++) {
        int count = countCategory(names[i]);
        int percent = total > 0 ? (count * 100) / total : 0;
        out << std::left << std::setw(22) << (names[i] + " Patients") << std::setw(10) << count
            << std::setw(10) << (utils::intToStr(percent) + "%") << std::string(percent / 4, '#')
            << "\n";
    }
    out << std::string(70, '-') << "\n";
    out << std::left << std::setw(22) << "Total" << total << "\n\n";
    out << "(each count is worked out by a recursive function)\n";
    return out.str();
}

std::string ReportGenerator::dailySummary(const std::string& date) const {
    if (!utils::isValidDate(date)) {
        throw model::InvalidInputException("Report date must be written as YYYY-MM-DD.");
    }
    std::stringstream out;
    out << header("REPORT 4: DAILY APPOINTMENT SUMMARY FOR " + date);

    const std::vector<model::Appointment*>& appts = database.getAppointments();
    int total = 0;
    int completed = 0;
    int pending = 0;
    int cancelled = 0;
    std::stringstream list;
    for (size_t i = 0; i < appts.size(); i++) {
        if (appts[i]->getDate() != date) {
            continue;
        }
        total++;
        if (appts[i]->getStatus() == "Completed") {
            completed++;
        } else if (appts[i]->getStatus() == "Cancelled") {
            cancelled++;
        } else {
            pending++;
        }
        model::PatientRecord* patient = database.findPatient(appts[i]->getPatientId());
        list << "  " << appts[i]->getTime() << "  #" << std::left << std::setw(5)
             << appts[i]->getNumber() << std::setw(24)
             << shorten(patient ? patient->getName() : appts[i]->getPatientId(), 22)
             << std::setw(14) << appts[i]->getDoctor() << appts[i]->getStatus() << "\n";
    }
    out << "Total Appointments: " << total << "\n";
    out << "Completed:          " << completed << "\n";
    out << "Pending:            " << pending << "\n";
    out << "Cancelled:          " << cancelled << "\n\n";
    if (total > 0) {
        out << "Appointments on this day:\n" << list.str();
    } else {
        out << "No appointments were booked for this day.\n";
    }
    return out.str();
}

// uses the base class pointer, each patient type works out its own fee
std::string ReportGenerator::feeSummary() const {
    std::stringstream out;
    out << header("REPORT 5: CONSULTATION FEES BY PATIENT TYPE");

    const std::vector<model::PatientRecord*>& patients = database.getPatients();
    int total = 0;
    out << std::left << std::setw(8) << "ID" << std::setw(26) << "Name" << std::setw(12)
        << "Category" << std::setw(16) << "Priority" << "Fee\n";
    for (size_t i = 0; i < patients.size(); i++) {
        model::PatientRecord* patient = patients[i];
        out << std::left << std::setw(8) << patient->getId() << std::setw(26)
            << shorten(patient->getName(), 24) << std::setw(12) << patient->getCategory()
            << std::setw(16) << patient->getPriorityLabel() << "N" << patient->getConsultationFee()
            << "\n";
        total += patient->getConsultationFee();
    }
    out << std::string(70, '-') << "\n";
    out << "Total expected fees: N" << total << "\n";
    return out.str();
}

}
