cmake_minimum_required(VERSION 3.10)
project(navidrome-export)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)

# Create the executable
add_executable(navidrome-export
    src/main.cpp
    src/utilities.cpp
    src/config.cpp
    src/test.cpp
)

# Link libraries
target_link_libraries(navidrome-export PRIVATE CURL::libcurl)

# Include directories for nlohmann/json (header-only)
target_include_directories(navidrome-export PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Installation
install(TARGETS navidrome-export DESTINATION ~/.local/bin)
