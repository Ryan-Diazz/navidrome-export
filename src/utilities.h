#include <iostream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <termios.h>
#include <unistd.h>

#ifndef UTILITIES_H
#define UTILITIES_H

namespace fs = std::filesystem;

struct Config {
    std::string navidromeUrl;
    std::string username;
    std::string password;
    CURL* curlHandle = nullptr;
};

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

std::string urlEncode(const std::string& value);

std::string stringToHex(const std::string& str);

std::string escapeCsv(const std::string& field);

fs::path getConfigDir();

void printHeader(const std::string& title);

std::string getPassword();

std::string makeApiRequest(const Config& config, const std::string& endpoint, const std::string& parameters);

#endif
