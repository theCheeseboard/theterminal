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
    terminalcontroller.cpp
    dialogs/busydialog.cpp
    dropdown.cpp
    print/printcontroller.cpp

    mainwindow.ui
    about.ui
    settingswindow.ui
    terminaltabber.ui
    terminalwidget.ui
    terminalstatus.ui
    dialogs/busydialog.ui

    icons.qrc

    defaults.conf)

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
    terminalcontroller.h
    dialogs/busydialog.h
    dropdown.h
    print/printcontroller.h)

add_executable(theterminal ${SOURCES} ${HEADERS})

set_target_properties(theterminal PROPERTIES
        MACOSX_BUNDLE TRUE
        CNTP_DATA_SUBDIR theterminal)

cntp_init(theterminal 17)
cntp_translate(theterminal)

target_link_libraries(theterminal Qt6::Widgets Qt6::DBus libcontemporary tttermwidget)
cntp_target_install_desktop_file_switch(theterminal
    com.vicr123.theterminal.desktop
    com.vicr123.theterminal_blueprint.desktop)

install(TARGETS theterminal
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION /Applications)
