#include <fstream>
#include <filesystem>

#include "../include/json.hpp"

#include "utilities.h"

namespace fs = std::filesystem;
using json = nlohmann::json;


// ============================================================================
// CONFIG MANAGEMENT
// ============================================================================


Config loadConfig() {
    fs::path configDir = getConfigDir();
    fs::path configFile = configDir / "config.json";
    
    if (!fs::exists(configFile)) {
        std::cerr << "Config file not found: " << configFile << std::endl;
        std::cerr << "Please run: navidrome-export init" << std::endl;
        throw std::runtime_error("Config file missing");
    }
    
    std::ifstream file(configFile);
    json configJson = json::parse(file);
    
    Config config;
    config.navidromeUrl = configJson["navidrome_url"];
    config.username = configJson["username"];
    config.password = configJson["password"];
    
    // Validate config
    if (config.navidromeUrl.empty() || config.username.empty() || 
        config.password.empty()) {
        throw std::runtime_error("Invalid config: missing required fields");
    }

    // Initialize CURL handle with connection pooling
    config.curlHandle = curl_easy_init();
    if (!config.curlHandle) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    // Enable connection reuse and keep-alive
    curl_easy_setopt(config.curlHandle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(config.curlHandle, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(config.curlHandle, CURLOPT_TCP_KEEPINTVL, 60L);
    
    return config;
}

// Add cleanup function (call before program exit)
void cleanupConfig(Config& config) {
    if (config.curlHandle) {
        curl_easy_cleanup(config.curlHandle);
        config.curlHandle = nullptr;
    }
}

void initializeConfig() {
    fs::path configDir = getConfigDir();
    fs::path configFile = configDir / "config.json";
    
    // Create directory if it doesn't exist
    if (!fs::exists(configDir)) {
        fs::create_directories(configDir);
        std::cout << "Created config directory: " << configDir << std::endl;
    }
    
    // Check if config already exists
    if (fs::exists(configFile)) {
        std::string response;
        std::cout << "Config file already exists: " << configFile << std::endl;
        std::cout << "Do you want to reconfigure? (y/n): ";
        std::getline(std::cin, response);
        
        if (response != "y" && response != "Y") {
            std::cout << "Configuration unchanged." << std::endl;
            return;
        }
    }
    
    // Prompt user for configuration
    std::string url, username, password;
    
    printHeader("navidrome-export Configuration");
    std::cout << "Enter Navidrome URL (e.g., http://192.168.1.100:4533): ";
    std::getline(std::cin, url);
    
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    
    std::cout << "Enter password: ";
    password = getPassword();
    
    if (url.empty() || username.empty() || password.empty()) {
        throw std::runtime_error("All fields are required");
    }
    
    // Create config JSON
    json configJson;
    configJson["navidrome_url"] = url;
    configJson["username"] = username;
    configJson["password"] = stringToHex(password);
    
    // Write to file with restricted permissions
    std::ofstream file(configFile);
    file << configJson.dump(4);
    file.close();
    
    // Set restrictive permissions (user read/write only)
    fs::permissions(configFile, fs::perms::owner_read | fs::perms::owner_write, 
                    fs::perm_options::replace);
    
    std::cout << "\nConfig saved to: " << configFile << std::endl;
    std::cout << "Permissions set to 0600 (user only)" << std::endl;
}

void resetConfig() {
    fs::path configDir = getConfigDir();
    fs::path configFile = configDir / "config.json";
    
    if (!fs::exists(configFile)) {
        std::cout << "No config file found to reset." << std::endl;
        return;
    }
    
    std::string response;
    std::cout << "\nAre you sure you want to delete the config file?" << std::endl;
    std::cout << "This cannot be undone. (y/N): ";
    std::getline(std::cin, response);
    
    if (response != "y" && response != "Y") {
        std::cout << "Reset cancelled." << std::endl;
        return;
    }
    
    try {
        fs::remove(configFile);
        std::cout << "Config file deleted: " << configFile << std::endl;
        std::cout << "Run 'navidrome-export init' to reconfigure." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error deleting config file: " << e.what() << std::endl;
    }
}