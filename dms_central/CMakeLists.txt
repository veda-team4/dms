cmake_minimum_required(VERSION 3.19)
project(dms_central LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

# OpenCV 추가
find_package(OpenCV REQUIRED)

qt_standard_project_setup()

qt_add_executable(dms_central
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    loginpage.h loginpage.cpp loginpage.ui
    mainpage.h mainpage.cpp mainpage.ui
    connectdialog.h connectdialog.cpp connectdialog.ui
)

target_link_libraries(dms_central
    PRIVATE
        Qt::Core
        Qt::Widgets
        ${OpenCV_LIBS}   # OpenCV 라이브러리 링크
)

# OpenCV include 경로 추가
target_include_directories(dms_central PRIVATE ${OpenCV_INCLUDE_DIRS})

include(GNUInstallDirs)

install(TARGETS dms_central
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET dms_central
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
