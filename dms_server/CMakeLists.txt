cmake_minimum_required(VERSION 3.10)
project(dms_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ignore compiler warning
add_compile_options(-Wno-psabi)

# include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Dlib
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/dlib ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(dlib STATIC IMPORTED)
set_target_properties(dlib PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/library/libdlib.a
)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Main executable
add_executable(dms_server
 src/main.cpp
 src/utils.cpp
 src/threads.cpp
 src/camsetpage.cpp
 src/calibratepage.cpp
 src/monitorpage.cpp
 src/startpage.cpp
 )

target_link_libraries(dms_server ${OpenCV_LIBS} dlib OpenSSL::SSL OpenSSL::Crypto)
