cmake_minimum_required(VERSION 3.21.0)

project(theterminal VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary REQUIRED)
find_package(tttermwidget REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp
    about.cpp
    models/colorschemeselectiondelegate.cpp
    nativeeventfilter.cpp
    settingswindow.cpp
    terminalpart.cpp
    terminaltabber.cpp
    terminalwidget.cpp
    terminalstatus.cpp
    history.cpp
    common.cpp
    terminalcontroller.cpp
    dialogs/busydialog.cpp
    print/printcontroller.cpp
    dialogs/remoteconnectionpopover.cpp

    mainwindow.ui
    about.ui
    settingswindow.ui
    terminaltabber.ui
    terminalwidget.ui
    terminalstatus.ui
    dialogs/busydialog.ui
    dialogs/remoteconnectionpopover.ui

    icons.qrc)

set(HEADERS
    mainwindow.h
    about.h
    models/colorschemeselectiondelegate.h
    nativeeventfilter.h
    settingswindow.h
    terminalpart.h
    terminaltabber.h
    terminalwidget.h
    terminalstatus.h
    history.h
    common.h
    terminalcontroller.h
    dialogs/busydialog.h
    print/printcontroller.h
    dialogs/remoteconnectionpopover.h)


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SOURCES ${SOURCES}
        dropdown.cpp
        dropdown.ui)
    set(HEADERS ${HEADERS} dropdown.h)
endif()

add_executable(theterminal ${SOURCES} ${HEADERS})

set_target_properties(theterminal PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.vicr123.theterminal
        CNTP_DATA_SUBDIR theterminal)

cntp_init(theterminal 17)
cntp_translate(theterminal)
cntp_target_name(theterminal "theTerminal")
cntp_mac_icon(theterminal
    ICON icon.icns
    BLUEPRINT_ICON icon-bp.icns)
cntp_target_desktop_file(theterminal
    DESKTOP_FILE com.vicr123.theterminal.desktop com.vicr123.theterminal.dropdown.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.theterminal_blueprint.desktop)
cntp_defaults_file(theterminal
    DEFAULTS_FILE theterminal.conf)

target_link_libraries(theterminal Qt6::Widgets Qt6::DBus libcontemporary tttermwidget)

install(TARGETS theterminal
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
