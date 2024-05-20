cmake_minimum_required(VERSION 3.20)
project(ExploreWindowsExecutableArtifacts)

list(APPEND CMAKE_PREFIX_PATH "C:\\Qt\\6.2.4\\msvc2019_64\\lib\\cmake")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(ewea main.cpp EWEAMainWindow.cpp)
target_include_directories(ewea PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ewea PUBLIC Qt6::Core Qt6::Widgets Qt6::Gui)