#include <iostream>
#include <string>
#include <curl/curl.h>

#include "../include/json.hpp"

#include "utilities.h"
#include "test.h"

using json = nlohmann::json;

static bool testConnectivity(const Config& config) {
    std::cout << "Test 1: Basic Connectivity" << std::endl;
    std::cout << "   Testing: HEAD " << config.navidromeUrl << "/" << std::endl;

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cout << "   FAILED: Could not initialize curl" << std::endl;
        return false;
    }

    std::string url = config.navidromeUrl + "/";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cout << "   FAILED: Cannot reach Navidrome (" << curl_easy_strerror(res) << ")" << std::endl;
        std::cout << "   Check:" << std::endl;
        std::cout << "     - Is Navidrome running?" << std::endl;
        std::cout << "     - Is the URL correct?" << std::endl;
        std::cout << "     - Is the port open?" << std::endl;
        std::cout << "     - Is there a firewall blocking?" << std::endl;
        return false;
    }

    std::cout << "   Navidrome server is reachable" << std::endl;
    return true;
}

static bool testApiPing(const Config& config) {
    std::cout << "Test 2: API Ping" << std::endl;
    std::cout << "   Testing: /rest/ping.view" << std::endl;

    std::string response = makeApiRequest(config, "ping.view", "");
    std::cout << "   Response: " << response << std::endl;

    json responseJson = json::parse(response);
    if (!responseJson.contains("subsonic-response")) {
        std::cout << "   FAILED: Unexpected response format" << std::endl;
        return false;
    }

    auto subsResponse = responseJson["subsonic-response"];
    if (subsResponse.value("status", "") == "ok") {
        std::cout << "   API is working" << std::endl;
        return true;
    }

    if (subsResponse.contains("error")) {
        std::string msg = subsResponse["error"].value("message", "Unknown error");
        std::cout << "   FAILED: " << msg << std::endl;
        std::cout << "   Check if username and password are correct" << std::endl;
    } else {
        std::cout << "   FAILED: Unexpected response" << std::endl;
        std::cout << "   Check if URL format is correct (no /rest/, no trailing slash)" << std::endl;
    }
    return false;
}

static bool testGetAlbum(const Config& config) {
    std::cout << "Test 3: Get getAlbumList2 (Sample)" << std::endl;
    std::cout << "   Testing: /rest/getAlbumList2.view" << std::endl;

    std::string response = makeApiRequest(config, "getAlbumList2.view", "type=alphabeticalByName&size=1");
    std::string preview = response.length() > 150 ? response.substr(0, 150) + "..." : response;
    std::cout << "   Response: " << response << std::endl;

    json responseJson = json::parse(response);
    if (!responseJson.contains("subsonic-response")) {
        std::cout << "   FAILED: Invalid response from getAlbumList2" << std::endl;
        std::cout << "   Check: API endpoint might have changed in your Navidrome version" << std::endl;
        return false;
    }

    auto subsResponse = responseJson["subsonic-response"];
    int albumCount = 0;
    if (subsResponse.contains("albumList2")) {
        auto albumList = subsResponse["albumList2"];
        if (albumList.contains("album")) {
            albumCount = albumList["album"].is_array() ? albumList["album"].size() : 1;
        }else if(albumList.is_array()) {
            albumCount = albumList.size();
        } else {
            albumCount = 0;
        }
    }

    std::cout << "   API returned albums (total: " << albumCount << ")" << std::endl;
    return true;
}

int runTestCommand(const Config& config) {
    printHeader("Navidrome Export Tool - Connectivity Diagnostic");

    std::cout << "Configuration:" << std::endl;
    std::cout << "   URL:      " << config.navidromeUrl << std::endl;
    std::cout << "   Username: " << config.username << std::endl;
    std::cout << "   Password: ****" << std::endl;
    std::cout << std::endl;

    if (!testConnectivity(config)) return 1;
    std::cout << std::endl;

    if (!testApiPing(config)) return 1;
    std::cout << std::endl;

    if (!testGetAlbum(config)) return 1;
    std::cout << std::endl;

    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                       ALL TESTS PASSED                             ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";

    std::cout << "navidrome-export is configured correctly!" << std::endl;
    std::cout << "You can now run: navidrome-export export" << std::endl;
    std::cout << std::endl;

    return 0;
}
