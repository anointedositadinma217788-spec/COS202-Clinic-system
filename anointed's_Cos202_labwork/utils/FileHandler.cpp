#include "FileHandler.h"
#include "Validator.h"
#include "../model/PatientFactory.h"
#include "../model/ClinicExceptions.h"
#include <fstream>
#include <cstdlib>
#include <direct.h>

namespace utils {

bool FileHandler::fileExists(const std::string& path) {
    std::ifstream file(path.c_str());
    return file.is_open();
}

void FileHandler::makeFolder(const std::string& path) {
    _mkdir(path.c_str());
}

std::vector<std::string> FileHandler::readLines(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        throw model::FileErrorException("Could not open the file:\n" + path);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        lines.push_back(line);
    }
    return lines;
}

void FileHandler::writeLines(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream file(path.c_str());
    if (!file.is_open()) {
        throw model::FileErrorException("Could not write to the file:\n" + path);
    }
    for (size_t i = 0; i < lines.size(); i++) {
        file << lines[i] << "\n";
    }
}

std::vector<std::string> FileHandler::split(const std::string& line, char separator) {
    std::vector<std::string> parts;
    std::string current = "";
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == separator) {
            parts.push_back(current);
            current = "";
        } else {
            current += line[i];
        }
    }
    parts.push_back(current);
    return parts;
}

// Category|ID|Name|Age|Contact|Extra1|Extra2
std::string FileHandler::patientToLine(const model::PatientRecord* patient) {
    return patient->getCategory() + "|" + patient->getId() + "|" + patient->getName() + "|" +
           intToStr(patient->getAge()) + "|" + patient->getContact() + "|" +
           patient->getExtra1() + "|" + patient->getExtra2();
}

model::PatientRecord* FileHandler::lineToPatient(const std::string& line) {
    std::vector<std::string> parts = split(line, '|');
    if (parts.size() < 5) {
        throw model::InvalidInputException("Damaged patient line: " + line);
    }
    std::string ageText = trim(parts[3]);
    if (!isWholeNumber(ageText) || ageText.size() > 3 || std::atoi(ageText.c_str()) > 120) {
        throw model::InvalidAgeException();
    }
    if (trim(parts[1]).empty() || trim(parts[2]).empty()) {
        throw model::InvalidInputException("Patient line is missing the ID or name.");
    }
    std::string extra1 = parts.size() > 5 ? parts[5] : "";
    std::string extra2 = parts.size() > 6 ? parts[6] : "";
    return model::createPatient(trim(parts[0]), trim(parts[1]), trim(parts[2]),
                                std::atoi(ageText.c_str()), trim(parts[4]), extra1, extra2);
}

// Number|PatientID|Doctor|Date|Time|Service|Status
std::string FileHandler::appointmentToLine(const model::Appointment* appt) {
    return intToStr(appt->getNumber()) + "|" + appt->getPatientId() + "|" + appt->getDoctor() + "|" +
           appt->getDate() + "|" + appt->getTime() + "|" + appt->getService() + "|" +
           appt->getStatus();
}

model::Appointment* FileHandler::lineToAppointment(const std::string& line) {
    std::vector<std::string> parts = split(line, '|');
    if (parts.size() != 7) {
        throw model::InvalidAppointmentException("Damaged appointment line: " + line);
    }
    if (!isWholeNumber(parts[0]) || parts[0].size() > 6 || !isValidDate(parts[3]) ||
        !isValidTime(parts[4])) {
        throw model::InvalidAppointmentException("Appointment line has bad values: " + line);
    }
    std::string status = parts[6];
    if (status != "Scheduled" && status != "Completed" && status != "Cancelled") {
        throw model::InvalidAppointmentException("Unknown appointment status: " + status);
    }
    return new model::Appointment(std::atoi(parts[0].c_str()), parts[1], parts[2], parts[3],
                                  parts[4], parts[5], status);
}

std::string FileHandler::csvField(const std::string& text) {
    bool needsQuotes = false;
    std::string result = "";
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == ',' || text[i] == '"') {
            needsQuotes = true;
        }
        if (text[i] == '"') {
            result += "\"\"";
        } else {
            result += text[i];
        }
    }
    if (needsQuotes) {
        return "\"" + result + "\"";
    }
    return result;
}

std::vector<std::string> FileHandler::splitCsv(const std::string& line) {
    std::vector<std::string> fields;
    std::string current = "";
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';
                i++;
            } else if (c == '"') {
                inQuotes = false;
            } else {
                current += c;
            }
        } else if (c == '"') {
            inQuotes = true;
        } else if (c == ',') {
            fields.push_back(current);
            current = "";
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

void FileHandler::exportPatientsCsv(const std::string& path,
                                    const std::vector<model::PatientRecord*>& patients) {
    std::vector<std::string> lines;
    lines.push_back("PatientID,Name,Age,Contact,Category,Extra1,Extra2");
    for (size_t i = 0; i < patients.size(); i++) {
        model::PatientRecord* p = patients[i];
        lines.push_back(csvField(p->getId()) + "," + csvField(p->getName()) + "," +
                        intToStr(p->getAge()) + "," + csvField(p->getContact()) + "," +
                        csvField(p->getCategory()) + "," + csvField(p->getExtra1()) + "," +
                        csvField(p->getExtra2()));
    }
    writeLines(path, lines);
}

// bad rows are counted in skipped instead of stopping the whole import
std::vector<model::PatientRecord*> FileHandler::importPatientsCsv(const std::string& path,
                                                                  int& skipped) {
    std::vector<std::string> lines = readLines(path);
    std::vector<model::PatientRecord*> result;
    skipped = 0;
    for (size_t i = 0; i < lines.size(); i++) {
        std::string line = trim(lines[i]);
        if (line.empty() || line.find("PatientID") == 0) {
            continue;
        }
        try {
            std::vector<std::string> f = splitCsv(line);
            if (f.size() < 5) {
                throw model::InvalidInputException("Not enough columns");
            }
            std::string joined = toUpper(trim(f[0])) + "|" + trim(f[1]) + "|" + trim(f[2]) +
                                 "|" + trim(f[3]) + "|" + (f.size() > 5 ? trim(f[5]) : "") +
                                 "|" + (f.size() > 6 ? trim(f[6]) : "");
            if (hasBadCharacters(joined)) {
                throw model::InvalidInputException("Row contains a | character");
            }
            result.push_back(lineToPatient(trim(f[4]) + "|" + joined));
        } catch (const std::exception&) {
            skipped++;
        }
    }
    return result;
}

}
