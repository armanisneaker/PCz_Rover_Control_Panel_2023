cmake_minimum_required(VERSION 3.5)

project(PCz_Rover_Control_Panel_2023 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        connection.h
        connection.cpp
        arm.h
        arm.cpp
        drive.h
        drive.cpp
        joystickwidget.h
        joystickwidget.cpp
        joystickdirectinput.h
        joystickdirectinput.cpp
        science.cpp
        science.h

        MapModule.h
        MapModule.cpp
        structures.h
        GPSList.cpp
        rover_pointer.h
        rover_pointer.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PCz_Rover_Control_Panel_2023
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PCz_Rover_Control_Panel_2023 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PCz_Rover_Control_Panel_2023 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PCz_Rover_Control_Panel_2023
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PCz_Rover_Control_Panel_2023 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(PCz_Rover_Control_Panel_2023 PRIVATE Qt${QT_VERSION_MAJOR}::Network)
target_link_libraries(PCz_Rover_Control_Panel_2023 PRIVATE dinput8)

set_target_properties(PCz_Rover_Control_Panel_2023 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS PCz_Rover_Control_Panel_2023
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PCz_Rover_Control_Panel_2023)
endif()
