#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <cstring>
#include <filesystem>

#include <termios.h>
#include <unistd.h>

#include "../include/json.hpp"

#include "utilities.h"
#include "config.h"
#include "test.h"

using json = nlohmann::json;

// ============================================================================
// Structs
// ============================================================================

struct Song {
    std::string title;
    std::string artist;
    std::string album;
    int year;
    std::string genre;
    std::string filePath;
};

// ============================================================================
// API COMMUNICATION
// ============================================================================
std::vector<std::string> fetchAlbums(const Config& config) {
    std::vector<std::string> albumIds;

    printHeader("Fetching albums from Navidrome...");

    int offset = 0;
    const int pageSize = 500; // subsonic maximum
    bool hasMore = true;
    int pageCount = 0;
    while (hasMore) {
        std::string endpoint = "getAlbumList2.view";
        std::string parameters = "type=alphabeticalByName&size=" + std::to_string(pageSize) + "&offset=" + std::to_string(offset);
        
        std::string response = makeApiRequest(config, endpoint, parameters);
        json responseJson = json::parse(response);
        
        pageCount++;

        // "subsonic-response" check
        if (!responseJson.contains("subsonic-response")){
            std::cout << "    [ERROR] No 'subsonic-response' in response" << std::endl;
            continue;
        }
        auto subsResponse = responseJson["subsonic-response"];

        // CHECK: albumList2
        if (!subsResponse.contains("albumList2")){
            std::cout << "    [ERROR] No 'albumList2' in response.";
            continue;
        }
        auto albumList = subsResponse["albumList2"];
        
        // ===========
        // subsonic / navidrome descripancy in api handling
        json albumsArray;
        if (albumList.contains("album")) {
            // Navidrome style: albumList2.album is the array
            albumsArray = albumList["album"];
        } else if (albumList.is_array()) {
            // Standard Subsonic style: albumList2 is direct array
            albumsArray = albumList;
        } else {
            hasMore = false;
            continue;
        }

        // no albums in array means the page was empty
        if (albumsArray.size() == 0) {
            hasMore = false;
            continue;
        }
        
        // Handle case where single album might not be in array
        if (!albumsArray.is_array()) {
            albumsArray = json::array({albumsArray});
        }
        
        std::cout << "    [Page " << pageCount << "] Found " << albumsArray.size() << " albums" << std::endl;
        
        for (const auto& albumJson : albumsArray) {
            std::string albumId = albumJson.value("id", "");
            if (!albumId.empty()) {
                albumIds.push_back(albumId);
            }
        }
        
        std::string totalAlbums = subsResponse.value("total", "Unknown");
        offset += pageSize;
        
        std::cout << "        Total so far: " << albumIds.size()<< "/" << totalAlbums << " albums" << std::endl;
    }
    
    std::cout << "Total albums found: " << albumIds.size() << std::endl;
    
    if (albumIds.empty()) {
        throw std::runtime_error("No albums found on server");
    }

    return albumIds;
}

std::vector<Song> fetchAllSongs(const Config& config, const std::vector<std::string> albumIds) {
    std::vector<Song> songs;

    // Fetch songs from each album using getAlbum (official Subsonic API)
    printHeader("Fetching songs from albums...");
    
    int songsFound = 0;
    for (size_t i = 0; i < albumIds.size(); i++) {
        std::string albumId = albumIds[i];
        std::string endpoint = "getAlbum.view";
        std::string parameters = "id=" + albumId;
        
        try {
            std::string response = makeApiRequest(config, endpoint, parameters);
            json responseJson = json::parse(response);
            
            if (!responseJson.contains("subsonic-response")){
                std::cout << "    [ERROR] No 'subsonic-response' in response for album " << albumId << std::endl;
                continue;
            }

            auto subsResponse = responseJson["subsonic-response"];
            if (!subsResponse.contains("album")) {
                std::cerr << "    [ERROR] No 'album' in 'subsonic-response' for album " << albumId << std::endl;
                std::cout << subsResponse.dump(4) << std::endl;
                continue;
            }

            auto albumData = subsResponse["album"];
            if (!albumData.contains("song")) {
                std::cout << "    [ERROR] No 'song' fields in album: " << albumId << std::endl;
                continue;
            }
            auto songsArray = albumData["song"];
            
            int albumSongCount = 0;
            if (songsArray.is_array()) {
                albumSongCount = songsArray.size();
            } else if (!songsArray.is_null()) {
                // Single song might not be in array
                albumSongCount = 1;
                songsArray = json::array({songsArray});
            }
            songsFound += albumSongCount;

            // process each song
            for (const auto& songJson : songsArray) {
                Song song;
                song.title = songJson.value("title", "Unknown");
                song.artist = songJson.value("artist", "Unknown Artist");
                song.album = songJson.value("album", "Unknown Album");
                song.year = songJson.value("year", 0);
                song.genre = songJson.value("genre", "Unknown");
                song.filePath = songJson.value("path", "");
                
                songs.push_back(song);
            }
            
            if ((i + 1) % 50 == 0 || i == albumIds.size() - 1) {
                std::cout << "    Processed " << (i + 1) << " / " << albumIds.size() << " albums (" << songs.size() << " songs found)" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "    [ERROR] Error fetching album " << albumId << ": " << e.what() << std::endl;
        }
    }
    
    if (songs.empty()) {
        throw std::runtime_error("No songs found");
    }

    std::cout << "Total songs extracted: " << songs.size() << std::endl;
    return songs;
}

// ============================================================================
// EXPORT FUNCTIONS
// ============================================================================
void exportToCsv(const std::vector<Song>& songs, const std::string& filename) {
    std::string outputFile = filename;
    if (filename.length() < 4 ||
        filename.substr(filename.length() - 4) != ".csv") {
        outputFile += ".csv";
    }

    std::cout << "Exporting " << songs.size() << " songs to: " << outputFile << "..." << std::endl;

    std::ofstream file(outputFile);
    
    // Write header
    file << "Title,Artist,Album,Year,Genre,FilePath" << std::endl;
    
    // Write songs
    for (const auto& song : songs) {
        file << escapeCsv(song.title) << ","
            << escapeCsv(song.artist) << ","
            << escapeCsv(song.album) << ","
            << song.year << ","
            << escapeCsv(song.genre) << ","
            << escapeCsv(song.filePath) << std::endl;
    }
    
    file.close();
    std::cout << "Exported " << songs.size() << " songs to: " << outputFile << std::endl;
}

void exportToTxt(const std::vector<Song>& songs, const std::string& filename) {
    std::string outputFile = filename;
    if (filename.length() < 4 ||
        filename.substr(filename.length() - 4) != ".txt") {
        outputFile += ".txt";
    }

    std::cout << "Exporting " << songs.size() << " songs to: " << outputFile << "..." << std::endl;


    std::ofstream file(outputFile);
    
    for (const auto& song : songs) {
        std::ostringstream line;
        line << song.artist << " - " << song.title;
        
        if (!song.album.empty()) {
            line << " [" << song.album << "]";
        }
        
        if (song.year > 0) {
            line << " (" << song.year << ")";
        }
        
        if (!song.filePath.empty()) {
            line << " [" << song.filePath << "]";
        }
        
        file << line.str() << std::endl;
    }
    
    file.close();
    std::cout << "Exported " << songs.size() << " songs to: " << outputFile << std::endl;
}

// ============================================================================
// HELP
// ============================================================================
void printHelp(const char* programName) {
    printHeader("navidrome-export CLI Help");
    std::cout << "Usage: " << programName << " <command> [OPTIONS]\n" << std::endl;
    std::cout << "COMMANDS:" << std::endl;
    std::cout << "  init                Initialize configuration (interactive setup)" << std::endl;
    std::cout << "  reset               Reset configuration" << std::endl;
    std::cout << "  test                Test Navidrome connectivity and API" << std::endl;
    std::cout << "  export              Export songs from Navidrome" << std::endl;
    std::cout << "  help                Show this help message" << std::endl;
    std::cout << "\nEXPORT COMMAND OPTIONS:" << std::endl;
    std::cout << "  --format <fmt>      Export format: csv, txt, or both (default: csv)" << std::endl;
    std::cout << "  --file, -f <file>   Output filename (default: navidrome_export)" << std::endl;
    std::cout << "\nEXAMPLES:" << std::endl;
    std::cout << "  " << programName << " init" << std::endl;
    std::cout << "  " << programName << " export" << std::endl;
    std::cout << "  " << programName << " reset" << std::endl;
    std::cout << "\nCONFIG:" << std::endl;
    std::cout << "  Location: " << getConfigDir() / "config.json" << std::endl;
    std::cout << std::endl;
}

void printExportHelp(const char* programName) {
    printHeader("navidrome-export export Help");
    std::cout << "Usage: " << programName << " export [OPTIONS]\n" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "  --format <fmt>      Format: csv, txt, or both (default: csv)" << std::endl;
    std::cout << "  --file, -f <file>   Output filename (default: navidrome_export)" << std::endl;
    std::cout << "\nEXAMPLES:" << std::endl;
    std::cout << "  " << programName << " export" << std::endl;
    std::cout << "  " << programName << " export --format csv" << std::endl;
    std::cout << "  " << programName << " export -f my_songs --format both" << std::endl;
    std::cout << "  " << programName << " export --format txt --file output" << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// MAIN
// ============================================================================
int exportCommand(int argc, char* argv[]) {
    std::string format = "csv";
    std::string outputFile = "navidrome_export";
    
    // Parse export options
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--format") {
            if (i + 1 < argc) {
                format = argv[++i];
                if (format != "csv" && format != "txt" && format != "both") {
                    throw std::runtime_error("Invalid format '" + format + "' \nMust be: csv, txt, or both");
                }
            } else {
                throw std::runtime_error("'--format' requires a format argument");
            }
        } else if (arg == "--file" || arg == "-f") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                throw std::runtime_error("'" + arg + "', Requires a filename");
            }
        } else if (arg == "--help" || arg == "-h") {
            printExportHelp(argv[0]);
            return 0;
        } else {
            printExportHelp(argv[0]);
            throw std::runtime_error("Unknown option '" + arg + "'");
        }
    }

    // =====================================
    printHeader("Loading configuration...");
    Config config = loadConfig();
    
    // =====================================
    try {
        printHeader("Connecting to Navidrome...");
        std::vector<std::string> albumIds = fetchAlbums(config);

        std::vector<Song> songs = fetchAllSongs(config, albumIds);
        
        printHeader("Exporting songs to File...");
        // Export based on format
        if (format == "both") {
            exportToCsv(songs, outputFile);
            exportToTxt(songs, outputFile);
        } else if (format == "txt") {
            exportToTxt(songs, outputFile);
        } else {
            exportToCsv(songs, outputFile);
        }
        
        printHeader("Export completed successfully!");
    } catch (const std::exception& e){
        cleanupConfig(config);
        throw;
    }

    cleanupConfig(config);
    return 0;
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            printHelp(argv[0]);
            return 1;
        }
        
        std::string command = argv[1];
        
        // ====== INIT COMMAND ======
        if (command == "init") {
            initializeConfig();
            return 0;
        }
        
        // ====== RESET COMMAND ======
        if (command == "reset") {
            resetConfig();
            return 0;
        }
        
        // ====== HELP COMMAND ======
        if (command == "help" || command == "--help" || command == "-h") {
            printHelp(argv[0]);
            return 0;
        }
        
        // ====== TEST COMMAND ======
        if (command == "test") {
            Config config = loadConfig();
            int result = runTestCommand(config);
            cleanupConfig(config);
            return result;
        }

        // ====== EXPORT COMMAND ======
        if (command == "export") {
            exportCommand(argc, argv);
            return 0;
        }
        
        // ====== UNKNOWN COMMAND ======
        std::cerr << "[ERROR]: Unknown command '" << command << "'" << std::endl;
        printHelp(argv[0]);
        return 1;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR]: " << e.what() << std::endl;
        return 1;
    }
}