cmake_minimum_required(VERSION 3.14)
project(dms_central LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets)

add_executable(dms_central
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    connectdialog.cpp connectdialog.h connectdialog.ui
    topframe.h topframe.cpp topframe.ui
    leftframe.h leftframe.cpp leftframe.ui
    centerframe.h centerframe.cpp centerframe.ui
    rightframe.h rightframe.cpp rightframe.ui
    resources/resources.qrc
    loginframe.h loginframe.cpp loginframe.ui
    camframe.h camframe.cpp camframe.ui
)

target_link_libraries(dms_central
    PRIVATE Qt6::Core Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets
)

target_include_directories(dms_central PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GNUInstallDirs)

install(TARGETS dms_central
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

#qt_generate_deploy_app_script(
#    TARGET dms_central
#    OUTPUT_SCRIPT deploy_script
#    NO_UNSUPPORTED_PLATFORM_ERROR
#)
#install(SCRIPT ${deploy_script})
