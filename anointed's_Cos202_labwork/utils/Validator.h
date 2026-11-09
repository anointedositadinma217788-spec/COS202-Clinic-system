#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <ctime>

namespace utils {

// small text and date helpers used all over the program
std::string trim(const std::string& text);
std::string toLower(std::string text);
std::string toUpper(std::string text);
std::string intToStr(int value);

bool isWholeNumber(const std::string& text);
bool hasBadCharacters(const std::string& text);

bool isValidDate(const std::string& date);
bool isValidTime(const std::string& time);
int timeToMinutes(const std::string& time);

std::string todayDate();
std::string currentTime();
std::string dateOf(time_t moment);
std::string clockOf(time_t moment);

}

#endif
