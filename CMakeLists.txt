cmake_minimum_required(VERSION 3.10)
project(transport_network)
set(TARGET transport_network)

set(CMAKE_CXX_STANDARD 23)

# Find package dependencies
find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# Set include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
        src/dot_parser.cxx
        src/city_display.cxx
        src/main.cxx
)

set(HEADERS
        include/dot_parser.hxx)

# Add executable
add_executable(${TARGET} ${SOURCES})

# Link libraries
target_link_libraries(${TARGET} PRIVATE
        CLI11::CLI11
        fmt::fmt
        spdlog::spdlog
)
