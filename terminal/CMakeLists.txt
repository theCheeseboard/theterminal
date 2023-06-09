cmake_minimum_required(VERSION 3.21.0)

project(theterminal VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)
find_package(libcontemporary REQUIRED)
find_package(tttermwidget REQUIRED)

set(SOURCES
    main.cpp
    mainwindow.cpp mainwindow.ui
    models/colorschemeselectiondelegate.cpp
    nativeeventfilter.cpp
    terminalpart.cpp
    terminaltabber.cpp terminaltabber.ui
    terminalwidget.cpp terminalwidget.ui
    terminalstatus.cpp terminalstatus.ui
    history.cpp
    common.cpp
    terminalcontroller.cpp
    dialogs/busydialog.cpp dialogs/busydialog.ui
    print/printcontroller.cpp
    dialogs/remoteconnectionpopover.cpp dialogs/remoteconnectionpopover.ui

    settingspanes/shellsettingspane.cpp settingspanes/shellsettingspane.ui
    settingspanes/textsettingspane.cpp settingspanes/textsettingspane.ui
    settingspanes/colourssettingspane.cpp settingspanes/colourssettingspane.ui
    settingspanes/scrollbacksettingspane.cpp settingspanes/scrollbacksettingspane.ui
    settingspanes/scrollingdownsettingspane.cpp settingspanes/scrollingdownsettingspane.ui
    settingspanes/activeterminalbellssettingspane.cpp settingspanes/activeterminalbellssettingspane.ui
    settingspanes/inactiveterminalbellssettingspane.cpp settingspanes/inactiveterminalbellssettingspane.ui
    settingspanes/dropdownterminalkeyboardbindingsettingspane.cpp settingspanes/dropdownterminalkeyboardbindingsettingspane.ui

    icons.qrc
)

set(HEADERS
    mainwindow.h
    models/colorschemeselectiondelegate.h
    nativeeventfilter.h
    terminalpart.h
    terminaltabber.h
    terminalwidget.h
    terminalstatus.h
    history.h
    common.h
    terminalcontroller.h
    dialogs/busydialog.h
    print/printcontroller.h
    dialogs/remoteconnectionpopover.h

    settingspanes/shellsettingspane.h
    settingspanes/textsettingspane.h
    settingspanes/colourssettingspane.h
    settingspanes/scrollbacksettingspane.h
    settingspanes/scrollingdownsettingspane.h
    settingspanes/activeterminalbellssettingspane.h
    settingspanes/inactiveterminalbellssettingspane.h
    settingspanes/dropdownterminalkeyboardbindingsettingspane.h
)


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

cntp_init(theterminal 20)
cntp_translate(theterminal)
cntp_target_name(theterminal "theTerminal")
cntp_app_icon(theterminal
    BASEICON icons/baseicon.svg
    BASEICON_MAC icons/baseicon-mac.svg)
cntp_target_desktop_file(theterminal
    DESKTOP_FILE com.vicr123.theterminal.desktop com.vicr123.theterminal.dropdown.desktop
    BLUEPRINT_DESKTOP_FILE com.vicr123.theterminal_blueprint.desktop)
cntp_defaults_file(theterminal
    DEFAULTS_FILE theterminal.conf)

target_link_libraries(theterminal Qt::Widgets Qt::DBus libcontemporary tttermwidget)

install(TARGETS theterminal
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
