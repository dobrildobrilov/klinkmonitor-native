cmake_minimum_required(VERSION 3.26)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" KLINKMONITOR_VERSION)
string(STRIP "${KLINKMONITOR_VERSION}" KLINKMONITOR_VERSION)

project(KLinkMonitorNative VERSION ${KLINKMONITOR_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)
find_package(KF6 REQUIRED COMPONENTS
    CoreAddons
    I18n
    StatusNotifierItem
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.h
    @ONLY
)

add_executable(klinkmonitor-native
    src/main.cpp
    src/app/App.cpp
    src/app/App.h
    src/tray/Tray.cpp
    src/tray/Tray.h
    src/monitor/InterfaceMonitor.cpp
    src/monitor/InterfaceMonitor.h
    src/database/HistoryDb.cpp
    src/database/HistoryDb.h
    src/dialogs/AboutDialog.cpp
    src/dialogs/AboutDialog.h
    src/dialogs/HistoryDialog.cpp
    src/dialogs/HistoryDialog.h
    src/dialogs/StatisticsDialog.cpp
    src/dialogs/StatisticsDialog.h
    src/dialogs/SettingsDialog.cpp
    src/dialogs/SettingsDialog.h
    src/icons/IconFactory.cpp
    src/icons/IconFactory.h
    src/settings/Settings.cpp
    src/settings/Settings.h
)
target_include_directories(klinkmonitor-native PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(klinkmonitor-native
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
    KF6::CoreAddons
    KF6::I18n
    KF6::StatusNotifierItem
)

install(TARGETS klinkmonitor-native DESTINATION bin)
install(FILES data/applications/klinkmonitor-native.desktop DESTINATION share/applications)
install(FILES data/icons/klinkmonitor-native.svg DESTINATION share/icons/hicolor/scalable/apps)
