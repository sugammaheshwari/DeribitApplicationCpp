
# Minimum version of CMake
cmake_minimum_required(VERSION 3.14)

# Project name and language
project(DeribitCpp LANGUAGES CXX)

# Set C++ standard (C++11 in this case)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable vcpkg integration
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Find the required libraries installed by vcpkg
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(websocketpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Add the executable (your main application)
#add_executable(DeribitOrderBookServer src/orderBookServer.cpp)
#add_executable(DeribitOrderBookClient src/orderBookClient.cpp)
#add_executable(testClient src/testClient.cpp)
#add_executable(testServer src/testServer.cpp)
add_executable(DeribitOrderManager src/orderManager.cpp)

# Link libraries to the project
#target_link_libraries(DeribitOrderBookServer PRIVATE nlohmann_json::nlohmann_json Boost::system websocketpp::websocketpp OpenSSL::SSL)
#target_link_libraries(DeribitOrderBookClient PRIVATE nlohmann_json::nlohmann_json Boost::system websocketpp::websocketpp OpenSSL::SSL)
#target_link_libraries(testClient PRIVATE nlohmann_json::nlohmann_json Boost::system websocketpp::websocketpp OpenSSL::SSL)
#target_link_libraries(testServer PRIVATE nlohmann_json::nlohmann_json Boost::system websocketpp::websocketpp OpenSSL::SSL)
target_link_libraries(DeribitOrderManager PRIVATE nlohmann_json::nlohmann_json Boost::system websocketpp::websocketpp OpenSSL::SSL)