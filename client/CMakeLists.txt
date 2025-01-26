# Set minimum Cmake version
cmake_minimum_required(VERSION 3.11)

# Start project and set its name
project(ticketmaster-but-worse LANGUAGES CXX)

#set default build type to Debug for development and debugging purposes
set(default_build_type "Debug")

set(CMAKE_BUILD_DIR "build")

# The following is required for language servers (like clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)

# Set the Qt version and required components
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Find and link qrencode library
find_package(PkgConfig REQUIRED)
pkg_check_modules(QREncode REQUIRED libqrencode)

# Add executable
add_executable(ticketmaster-but-worse src/main.cpp src/QRCodeWindow.cpp)


# Link Qt libraries to the ticketmaster-but-worse app
target_link_libraries(ticketmaster-but-worse Qt5::Widgets)


#add local hpp libs
add_library(server-connection src/server-connection.cpp)
target_link_libraries(ticketmaster-but-worse server-connection)
add_library(ticket src/ticket.cpp)
target_link_libraries(ticketmaster-but-worse ticket)
add_library(exceptions-base src/exceptions-base.cpp)
target_link_libraries(ticketmaster-but-worse exceptions-base)

#add json library
add_subdirectory(submodules/json)
target_link_libraries(ticket nlohmann_json::nlohmann_json)
target_link_libraries(server-connection nlohmann_json::nlohmann_json)

#add httplib library
add_subdirectory(submodules/cpp-httplib)
target_link_libraries(server-connection httplib)

#add libcotp library
set(HMAC_WRAPPER "openssl" CACHE STRING "tell libcotp to use openssl")
add_subdirectory(submodules/libcotp)
target_link_libraries(ticket cotp)

# Link qrencode to the executable
add_subdirectory(submodules/qrencode)
target_link_libraries(ticketmaster-but-worse qrencode)
