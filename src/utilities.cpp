#include <iostream>
#include <sstream>
#include <string>
#include <curl/curl.h>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <termios.h>
#include <unistd.h>

#include "../include/json.hpp"

#include "utilities.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================
// Callback for cURL to capture response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// URL encode string
std::string urlEncode(const std::string& value) {
    CURL* curl = curl_easy_init();
    if (!curl) return value;
    
    char* encoded = curl_easy_escape(curl, value.c_str(), value.length());
    std::string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    
    return result;
}

// Convert string to hex (for Subsonic API password encoding)
std::string stringToHex(const std::string& str) {
    std::ostringstream oss;
    for (unsigned char c : str) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

// Escape CSV field
std::string escapeCsv(const std::string& field) {
    std::string result = field;
    
    // If field contains comma, newline, or quote, wrap in quotes
    if (field.find(',') != std::string::npos ||
        field.find('\n') != std::string::npos ||
        field.find('"') != std::string::npos) {
        
        // Escape quotes by doubling them
        size_t pos = 0;
        while ((pos = result.find('"', pos)) != std::string::npos) {
            result.insert(pos, 1, '"');
            pos += 2;
        }
        result = "\"" + result + "\"";
    }
    
    return result;
}

// Get config directory
fs::path getConfigDir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("HOME environment variable not set");
    }
    
    fs::path configDir = fs::path(home) / ".config" / "navidrome-export";
    return configDir;
}

// print the header of a step
void printHeader(const std::string& title){
    std::cout << "\n==== " << title << " ====" <<std::endl;
}

// make the password entry field secure
std::string getPassword() {
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::string password;
    std::getline(std::cin, password);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << '\n';

    return password;
}

std::string makeApiRequest(const Config& config, const std::string& endpoint, const std::string& parameters = "") {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    std::string fullUrl = config.navidromeUrl + "/rest/" + endpoint + 
                        "?u=" + urlEncode(config.username) + 
                        "&p=enc:" + urlEncode(config.password) + 
                        "&c=navidrome-export&f=json&v=1.12.0&" + parameters ;

    std::string response;
    curl_easy_setopt(config.curlHandle, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(config.curlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(config.curlHandle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(config.curlHandle, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(config.curlHandle);
    
    if (res != CURLE_OK) {
        std::string error = std::string(curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL error: " + error);
    }
    
    long httpCode = 0;
    curl_easy_getinfo(config.curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);    
    if (httpCode != 200) {
        throw std::runtime_error("HTTP error: " + std::to_string(httpCode));
    }
    
    return response;
}