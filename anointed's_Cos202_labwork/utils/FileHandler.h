#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "../model/PatientRecord.h"
#include "../model/Appointment.h"
#include <string>
#include <vector>

namespace utils {

// reading and writing the text data files (fields split with |)
class FileHandler {
public:
    static bool fileExists(const std::string& path);
    static void makeFolder(const std::string& path);

    static std::vector<std::string> readLines(const std::string& path);
    static void writeLines(const std::string& path, const std::vector<std::string>& lines);
    static std::vector<std::string> split(const std::string& line, char separator);

    static std::string patientToLine(const model::PatientRecord* patient);
    static model::PatientRecord* lineToPatient(const std::string& line);
    static std::string appointmentToLine(const model::Appointment* appt);
    static model::Appointment* lineToAppointment(const std::string& line);

    static void exportPatientsCsv(const std::string& path,
                                  const std::vector<model::PatientRecord*>& patients);
    static std::vector<model::PatientRecord*> importPatientsCsv(const std::string& path,
                                                                int& skipped);

private:
    static std::vector<std::string> splitCsv(const std::string& line);
    static std::string csvField(const std::string& text);
};

}

#endif
