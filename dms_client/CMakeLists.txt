cmake_minimum_required(VERSION 3.5)

project(dms_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        src/main.cpp
        src/basepage.cpp include/basepage.h
        src/mainwindow.cpp include/mainwindow.h ui/mainwindow.ui
        src/startpage.cpp include/startpage.h ui/startpage.ui
        src/camsetpage.cpp include/camsetpage.h ui/camsetpage.ui
        src/calibratepage.cpp include/calibratepage.h ui/calibratepage.ui
        src/monitorpage.cpp include/monitorpage.h ui/monitorpage.ui
        src/reportpage.cpp include/reportpage.h ui/reportpage.ui
        src/utils.cpp include/utils.h
        src/led.cpp include/led.h
        src/speaker.cpp include/speaker.h
        src/bluetooth.cpp include/bluetooth.h
        src/gps.cpp include/gps.h
        src/co2.cpp include/co2.h
        src/osrm.cpp include/osrm.h
        include/json.hpp
        include/protocols.h
        resources/resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(dms_client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET dms_client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(dms_client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(dms_client
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# OpenSSL
find_package(OpenSSL REQUIRED)

# Curl
find_package(CURL REQUIRED)

# Chart
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Charts REQUIRED)

target_include_directories(dms_client PRIVATE include)

target_link_libraries(dms_client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    Qt${QT_VERSION_MAJOR}::Charts
)

set_target_properties(dms_client PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS dms_client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dms_client)
endif()
