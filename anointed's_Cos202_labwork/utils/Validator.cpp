#include "Validator.h"
#include <sstream>
#include <cstdio>
#include <cctype>
#include <cstdlib>

namespace utils {

std::string trim(const std::string& text) {
    size_t start = 0;
    while (start < text.size() && std::isspace((unsigned char)text[start])) {
        start++;
    }
    size_t end = text.size();
    while (end > start && std::isspace((unsigned char)text[end - 1])) {
        end--;
    }
    return text.substr(start, end - start);
}

std::string toLower(std::string text) {
    for (size_t i = 0; i < text.size(); i++) {
        text[i] = (char)std::tolower((unsigned char)text[i]);
    }
    return text;
}

std::string toUpper(std::string text) {
    for (size_t i = 0; i < text.size(); i++) {
        text[i] = (char)std::toupper((unsigned char)text[i]);
    }
    return text;
}

std::string intToStr(int value) {
    std::stringstream out;
    out << value;
    return out.str();
}

bool isWholeNumber(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    for (size_t i = 0; i < text.size(); i++) {
        if (!std::isdigit((unsigned char)text[i])) {
            return false;
        }
    }
    return true;
}

// the | character is used to split fields in the data files
bool hasBadCharacters(const std::string& text) {
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '|' || text[i] == '\n' || text[i] == '\r') {
            return true;
        }
    }
    return false;
}

// date must look like 2026-11-09
bool isValidDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    std::string yearText = date.substr(0, 4);
    std::string monthText = date.substr(5, 2);
    std::string dayText = date.substr(8, 2);
    if (!isWholeNumber(yearText) || !isWholeNumber(monthText) || !isWholeNumber(dayText)) {
        return false;
    }
    int year = std::atoi(yearText.c_str());
    int month = std::atoi(monthText.c_str());
    int day = std::atoi(dayText.c_str());
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1) {
        return false;
    }
    int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leapYear) {
        daysInMonth[1] = 29;
    }
    return day <= daysInMonth[month - 1];
}

// time must look like 09:30 (24 hour clock)
bool isValidTime(const std::string& time) {
    if (time.size() != 5 || time[2] != ':') {
        return false;
    }
    std::string hourText = time.substr(0, 2);
    std::string minuteText = time.substr(3, 2);
    if (!isWholeNumber(hourText) || !isWholeNumber(minuteText)) {
        return false;
    }
    int hour = std::atoi(hourText.c_str());
    int minute = std::atoi(minuteText.c_str());
    return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

int timeToMinutes(const std::string& time) {
    if (!isValidTime(time)) {
        return -1;
    }
    return std::atoi(time.substr(0, 2).c_str()) * 60 + std::atoi(time.substr(3, 2).c_str());
}

std::string dateOf(time_t moment) {
    tm* local = localtime(&moment);
    char buffer[16];
    sprintf(buffer, "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
    return buffer;
}

std::string clockOf(time_t moment) {
    tm* local = localtime(&moment);
    char buffer[16];
    sprintf(buffer, "%02d:%02d", local->tm_hour, local->tm_min);
    return buffer;
}

std::string todayDate() {
    return dateOf(time(NULL));
}

std::string currentTime() {
    return clockOf(time(NULL));
}

}
